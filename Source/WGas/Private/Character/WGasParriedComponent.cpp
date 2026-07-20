// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/WGasParriedComponent.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "WGasGameplayTags.h"
#include "Character/WGasCharacterBase.h"
#include "Character/EnemyComponent/BossAIPauseComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UWGasParriedComponent::UWGasParriedComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWGasParriedComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<AWGasCharacterBase>(GetOwner());
}

void UWGasParriedComponent::SuspendActions()
{
	AWGasCharacterBase* Character = OwnerCharacter.Get();
	if (!Character)
	{
		return;
	}

	if (UCharacterMovementComponent* Move = Character->GetCharacterMovement())
	{
		Move->StopMovementImmediately();
		Move->DisableMovement();
		bAppliedMovementLock = true;
	}

	if (UBossAIPauseComponent* BossPause = Character->FindComponentByClass<UBossAIPauseComponent>())
	{
		BossPause->StopMovement();
		bBossBrainPaused = BossPause->PauseBrain(BrainPauseReason);
		BossPause->DisableMovement();
		BossPause->SetBlackboardBool(ParriedBlackboardKey, true);
		bBossMovementDisabled = true;
	}

	if (!bBossBrainPaused)
	{
		if (AAIController* AIC = Cast<AAIController>(Character->GetController()))
		{
			AIC->StopMovement();
			if (UBrainComponent* Brain = AIC->BrainComponent)
			{
				Brain->StopLogic(BrainPauseReason.ToString());
				bAppliedDirectBrainStop = true;
			}
		}
	}
}

void UWGasParriedComponent::ResumeActions()
{
	AWGasCharacterBase* Character = OwnerCharacter.Get();
	if (!Character)
	{
		return;
	}

	if (UBossAIPauseComponent* BossPause = Character->FindComponentByClass<UBossAIPauseComponent>())
	{
		if (bBossMovementDisabled)
		{
			BossPause->SetBlackboardBool(ParriedBlackboardKey, false);
			BossPause->RestoreMovement();
			bBossMovementDisabled = false;
		}

		if (bBossBrainPaused)
		{
			BossPause->ResumeBrain(BrainPauseReason, true);
			bBossBrainPaused = false;
		}
	}

	if (bAppliedDirectBrainStop)
	{
		if (AAIController* AIC = Cast<AAIController>(Character->GetController()))
		{
			if (UBrainComponent* Brain = AIC->BrainComponent)
			{
				Brain->ResumeLogic(BrainPauseReason.ToString());
			}
		}
		bAppliedDirectBrainStop = false;
	}

	if (bAppliedMovementLock)
	{
		if (UCharacterMovementComponent* Move = Character->GetCharacterMovement())
		{
			if (Move->MovementMode == MOVE_None)
			{
				Move->SetMovementMode(MOVE_Walking);
			}
		}
		bAppliedMovementLock = false;
	}
}

bool UWGasParriedComponent::ShouldSkipParried() const
{
	const AWGasCharacterBase* Character = OwnerCharacter.Get();
	if (!Character)
	{
		return true;
	}

	UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
	if (!ASC)
	{
		return false;
	}

	const FWGasGameplayTags& Tags = FWGasGameplayTags::Get();
	if (Tags.State_Parried.IsValid() && ASC->HasMatchingGameplayTag(Tags.State_Parried))
	{
		return true;
	}
	if (Tags.State_Boss_PoiseBroken.IsValid() && ASC->HasMatchingGameplayTag(Tags.State_Boss_PoiseBroken))
	{
		return true;
	}
	if (Tags.State_Boss_PhaseTransition.IsValid() && ASC->HasMatchingGameplayTag(Tags.State_Boss_PhaseTransition))
	{
		return true;
	}
	if (Tags.State_Boss_Invulnerable.IsValid() && ASC->HasMatchingGameplayTag(Tags.State_Boss_Invulnerable))
	{
		return true;
	}

	return false;
}

void UWGasParriedComponent::ApplyParriedTag()
{
	AWGasCharacterBase* Character = OwnerCharacter.Get();
	if (!Character)
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent())
	{
		const FGameplayTag& Tag = FWGasGameplayTags::Get().State_Parried;
		if (Tag.IsValid())
		{
			ASC->AddLooseGameplayTag(Tag);
		}
	}
}

void UWGasParriedComponent::RemoveParriedTag()
{
	AWGasCharacterBase* Character = OwnerCharacter.Get();
	if (!Character)
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent())
	{
		const FGameplayTag& Tag = FWGasGameplayTags::Get().State_Parried;
		if (Tag.IsValid())
		{
			ASC->RemoveLooseGameplayTag(Tag);
		}
	}
}

void UWGasParriedComponent::CancelActiveAttacks()
{
	AWGasCharacterBase* Character = OwnerCharacter.Get();
	if (!Character)
	{
		return;
	}

	UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	const FWGasGameplayTags& Tags = FWGasGameplayTags::Get();
	FGameplayTagContainer CancelTags;
	if (Tags.State_Boss_Attacking.IsValid())
	{
		CancelTags.AddTag(Tags.State_Boss_Attacking);
	}
	if (Tags.Abilities_Attack_Melee.IsValid())
	{
		CancelTags.AddTag(Tags.Abilities_Attack_Melee);
	}
	if (!CancelTags.IsEmpty())
	{
		ASC->CancelAbilities(&CancelTags);
	}

	if (Character->CombatComponent)
	{
		Character->CombatComponent->EndWeaponSweep();
	}
}

void UWGasParriedComponent::PlayParriedMontage()
{
	AWGasCharacterBase* Character = OwnerCharacter.Get();
	if (!Character || !ParriedMontage)
	{
		return;
	}

	USkeletalMeshComponent* Mesh = Character->GetMesh();
	UAnimInstance* Anim = Mesh ? Mesh->GetAnimInstance() : nullptr;
	if (!Anim)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] 无 AnimInstance，跳过 ParriedMontage"), *GetNameSafe(Character));
		return;
	}

	const float PlayedDuration = Character->PlayAnimMontage(ParriedMontage);
	if (PlayedDuration <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] PlayAnimMontage 失败: %s（检查 Skeleton / Slot）"),
			*GetNameSafe(Character), *GetNameSafe(ParriedMontage));
		return;
	}

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UWGasParriedComponent::OnParriedMontageEnded);
	Anim->Montage_SetEndDelegate(EndDelegate, ParriedMontage);
}

void UWGasParriedComponent::StartPostParriedIdleTimer()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float IdleDuration = FMath::Max(PostParriedIdleDuration, 0.f);
	if (IdleDuration <= KINDA_SMALL_NUMBER)
	{
		ExitParried();
		return;
	}

	World->GetTimerManager().ClearTimer(FallbackTimerHandle);
	World->GetTimerManager().SetTimer(
		FallbackTimerHandle, this, &UWGasParriedComponent::ExitParried,
		IdleDuration, false);
}

void UWGasParriedComponent::ClearFallbackTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FallbackTimerHandle);
	}
}

void UWGasParriedComponent::OnParriedMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != ParriedMontage || !bParriedActive)
	{
		return;
	}

	StartPostParriedIdleTimer();
}

bool UWGasParriedComponent::EnterParried(AActor* ParryInstigator)
{
	if (bParriedActive || !OwnerCharacter.IsValid())
	{
		return false;
	}
	if (!bCanBeParried || ShouldSkipParried())
	{
		return false;
	}

	bParriedActive = true;
	ApplyParriedTag();
	CancelActiveAttacks();
	SuspendActions();
	PlayParriedMontage();

	// 无蒙太奇 / 播放失败：用 Fallback 时长；有蒙太奇则播完后走 PostParriedIdleDuration
	if (!ParriedMontage)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				FallbackTimerHandle, this, &UWGasParriedComponent::ExitParried,
				FallbackParriedDuration, false);
		}
	}
	else if (AWGasCharacterBase* Character = OwnerCharacter.Get())
	{
		if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			if (UAnimInstance* Anim = Mesh->GetAnimInstance())
			{
				if (!Anim->Montage_IsPlaying(ParriedMontage))
				{
					StartPostParriedIdleTimer();
				}
			}
		}
	}

	OnParriedStarted.Broadcast(ParryInstigator);
	return true;
}

void UWGasParriedComponent::ExitParried()
{
	if (!bParriedActive)
	{
		return;
	}

	bParriedActive = false;
	ClearFallbackTimer();

	if (AWGasCharacterBase* Character = OwnerCharacter.Get())
	{
		if (ParriedMontage)
		{
			Character->StopAnimMontage(ParriedMontage);
		}
	}

	RemoveParriedTag();
	ResumeActions();
	OnParriedEnded.Broadcast(nullptr);
}

void UWGasParriedComponent::ForceExitParried()
{
	ClearFallbackTimer();
	if (AWGasCharacterBase* Character = OwnerCharacter.Get())
	{
		if (ParriedMontage)
		{
			Character->StopAnimMontage(ParriedMontage);
		}
	}
	ExitParried();
}
