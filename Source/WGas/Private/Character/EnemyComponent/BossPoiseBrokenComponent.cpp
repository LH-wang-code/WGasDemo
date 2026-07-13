// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/EnemyComponent/BossPoiseBrokenComponent.h"

#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "WGasGameplayTags.h"
#include "AbilitySystem/WGasAbilitySystemComponent.h"
#include "AbilitySystem/WGasAttributeSet.h"
#include "Character/WGasCharacterEnemy.h"
#include "Character/EnemyComponent//BossAIPauseComponent.h"
#include "Character/EnemyComponent/BossPhaseTransitionComponent.h"
UBossPoiseBrokenComponent::UBossPoiseBrokenComponent()
{

	PrimaryComponentTick.bCanEverTick = true;

}

void UBossPoiseBrokenComponent::Initialize(AWGasCharacterEnemy* InOwner, UBossAIPauseComponent* InAIPause,
	UBossPhaseTransitionComponent* InPhase)
{
	OwnerEnemy = InOwner;
	AIPause = InAIPause;
	Phase = InPhase;
}

void UBossPoiseBrokenComponent::BindDelegates()
{
	if (bDelegatesBound)return;
	
	AWGasCharacterEnemy* Enemy = OwnerEnemy.Get();
	if (!Enemy || !Enemy->GetAttributeSet()) return;
	if (UWGasAttributeSet* AS = Cast<UWGasAttributeSet>(Enemy->GetAttributeSet()))
	{
		AS->OnPoiseBroken.AddUObject(this, &UBossPoiseBrokenComponent::EnterPoiseBroken);
		AS->OnPoiseRecovered.AddUObject(this, &UBossPoiseBrokenComponent::RequestPoiseBrokenExit);
		bDelegatesBound = true;
	}
}

bool UBossPoiseBrokenComponent::IsPoiseBrokenActive() const
{
	AWGasCharacterEnemy* Enemy = OwnerEnemy.Get();
	if (!Enemy)return false;
	UWGasAbilitySystemComponent* ASC = Cast<UWGasAbilitySystemComponent>(Enemy->GetAbilitySystemComponent());
	if (!ASC)return false;
	const FGameplayTag& Tag = FWGasGameplayTags::Get().State_Boss_PoiseBroken;
	return Tag.IsValid() && ASC->HasMatchingGameplayTag(Tag);
}


void UBossPoiseBrokenComponent::ForceCleanupWithoutResumeBrain()
{
	CleanupWithoutResumeBrain();
	// 转阶段时 Poise 的 Brain 暂停被 Phase 接管，需要清掉 Poise 的 pause reason
	if (UBossAIPauseComponent* AIPauseComp = AIPause.Get())
	{
		if (AIPauseComp->IsBrainPaused(TEXT("PoiseBroken")))
		{
			AIPauseComp->ClearPauseReason(TEXT("PoiseBroken"));
		}
	}
}

void UBossPoiseBrokenComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!bPoiseBrokenRotationLocked)
	{
		return;
	}
	AWGasCharacterEnemy* Enemy = OwnerEnemy.Get();
	if (!Enemy)return;
	const FRotator LockedRotation(CachedLockedRotation.Pitch, CachedLockedRotation.Yaw, CachedLockedRotation.Roll);
	if (!Enemy->GetActorRotation().Equals(LockedRotation, 0.5f))
	{
		Enemy->SetActorRotation(LockedRotation);
	}
	if (AAIController* AIC = Cast<AAIController>(Enemy->GetController()))
	{
		AIC->SetControlRotation(LockedRotation);
		AIC->ClearFocus(EAIFocusPriority::Gameplay);
		AIC->ClearFocus(EAIFocusPriority::Move);
	}
}

void UBossPoiseBrokenComponent::EnterPoiseBroken()
{
	AWGasCharacterEnemy* Enemy = OwnerEnemy.Get();
	UBossAIPauseComponent* AIPauseComp = AIPause.Get();
	if (!Enemy || !AIPauseComp)return;
	UWGasAbilitySystemComponent* ASC = Cast<UWGasAbilitySystemComponent>(Enemy->GetAbilitySystemComponent());
	if (!ASC || ShouldSkipPoise())return;
	const FWGasGameplayTags& WGasTags = FWGasGameplayTags::Get();
	if (WGasTags.State_Boss_Attacking.IsValid())
	{
		FGameplayTagContainer CancelTags;
		CancelTags.AddTag(WGasTags.State_Boss_Attacking);
		ASC->CancelAbilities(&CancelTags);
	}
	AIPauseComp->ResetDefaultMovement();
	if (Enemy->CombatComponent)
	{
		Enemy->CombatComponent->EndWeaponSweep();
	}
	AIPauseComp->StopMovement();
	AIPauseComp->PauseBrain(TEXT("PoiseBroken"));
	AIPauseComp->DisableMovement();
	LockRotation();
	if (USkeletalMeshComponent* MeshComp = Enemy->GetMesh())
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			AnimInstance->Montage_Stop(0.2f);
		}
	}
	if (PoiseBrokenEffectClass)
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		Context.AddSourceObject(Enemy);
		const FGameplayEffectSpecHandle SpecHandle =
			ASC->MakeOutgoingSpec(PoiseBrokenEffectClass, 1.f, Context);
		PoiseBrokenEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
	if (PoiseRegenEffectClass)
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		Context.AddSourceObject(Enemy);
		const FGameplayEffectSpecHandle SpecHandle =
			ASC->MakeOutgoingSpec(PoiseRegenEffectClass, 1.f, Context);
		PoiseRegenEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
	if (PoiseBrokenMontage)
	{
		Enemy->PlayAnimMontage(PoiseBrokenMontage);
		if (USkeletalMeshComponent* MeshComp = Enemy->GetMesh())
		{
			if (UAnimInstance* Anim = MeshComp->GetAnimInstance())
			{
				FOnMontageEnded EndDelegate;
				EndDelegate.BindUObject(this, &UBossPoiseBrokenComponent::OnPoiseBrokenMontageEnded);
				Anim->Montage_SetEndDelegate(EndDelegate, PoiseBrokenMontage);
			}
		}
	}
	AIPauseComp->SetBlackboardBool(PoiseBrokenBlackboardKey, true);
	bExitPending = false;
}

void UBossPoiseBrokenComponent::ExitPoiseBroken()
{
	UBossAIPauseComponent* AIPauseComp = AIPause.Get();
	if (!AIPauseComp)
	{
		return;
	}
	CleanupWithoutResumeBrain();
	AIPauseComp->ResetDefaultMovement();
	AIPauseComp->CleanupPathAndFocus();
	AIPauseComp->ResumeBrain(TEXT("PoiseBroken"));
}

void UBossPoiseBrokenComponent::RequestPoiseBrokenExit()
{
	if (bExitPending)return;
	if (const UBossPhaseTransitionComponent* PhaseComp = Phase.Get())
	{
		if (PhaseComp->IsTransitionActive())
		{
			return;
		}
	}
	bExitPending = true;
	AWGasCharacterEnemy* Enemy = OwnerEnemy.Get();
	if (!Enemy)return;
	UWGasAbilitySystemComponent* ASC = Cast<UWGasAbilitySystemComponent>(Enemy->GetAbilitySystemComponent());
	if (ASC && PoiseRegenEffectHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(PoiseRegenEffectHandle);
		PoiseRegenEffectHandle.Invalidate();
	}
	USkeletalMeshComponent* MeshComp = Enemy->GetMesh();
	UAnimInstance* Anim = MeshComp ? MeshComp->GetAnimInstance() : nullptr;
	if (Anim && PoiseBrokenMontage && Anim->Montage_IsPlaying(PoiseBrokenMontage))
	{
		Anim->Montage_JumpToSection(PoiseBrokenEndSection, PoiseBrokenMontage);
		return;
	}
	bExitPending = false;
	ExitPoiseBroken();
}

void UBossPoiseBrokenComponent::CleanupWithoutResumeBrain()
{
	AWGasCharacterEnemy* Enemy = OwnerEnemy.Get();
	UBossAIPauseComponent* AIPauseComp = AIPause.Get();
	if (!Enemy)return;
	UWGasAbilitySystemComponent* ASC = Cast<UWGasAbilitySystemComponent>(Enemy->GetAbilitySystemComponent());
	if (ASC)
	{
		if (PoiseBrokenEffectHandle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(PoiseBrokenEffectHandle);
			PoiseBrokenEffectHandle.Invalidate();
		}
		if (PoiseRegenEffectHandle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(PoiseRegenEffectHandle);
			PoiseRegenEffectHandle.Invalidate();
		}
		const FGameplayTag& PoiseBrokenTag = FWGasGameplayTags::Get().State_Boss_PoiseBroken;
		if (PoiseBrokenTag.IsValid())
		{
			FGameplayTagContainer Tags;
			Tags.AddTag(PoiseBrokenTag);
			ASC->RemoveActiveEffectsWithGrantedTags(Tags);
		}
	}
	if (AIPauseComp)
	{
		AIPauseComp->SetBlackboardBool(PoiseBrokenBlackboardKey, false);
	}
	UnlockRotation();
	if (AIPauseComp)
	{
		AIPauseComp->RestoreMovement();
	}
	bExitPending = false;
}

void UBossPoiseBrokenComponent::LockRotation()
{
	AWGasCharacterEnemy* Enemy = OwnerEnemy.Get();
	if (!Enemy)return;
	CachedLockedRotation = Enemy->GetActorRotation();
	bPoiseBrokenRotationLocked = true;
	if (!bRotationLocked)
	{
		bCachedUseControllerRotationYaw = Enemy->bUseControllerRotationYaw;
		bRotationLocked = true;
	}
	Enemy->bUseControllerRotationYaw = false;
	if (UCharacterMovementComponent* Movement = Enemy->GetCharacterMovement())
	{
		bCachedUseControllerDesiredRotation = Movement->bUseControllerDesiredRotation;
		bCachedOrientRotationToMovement = Movement->bOrientRotationToMovement;
		CachedRotationRate = Movement->RotationRate;
		Movement->bUseControllerDesiredRotation = false;
		Movement->bOrientRotationToMovement = false;
		Movement->RotationRate = FRotator::ZeroRotator;
	}
	if (AAIController* AIC = Cast<AAIController>(Enemy->GetController()))
	{
		AIC->SetControlRotation(CachedLockedRotation);
	}
}

void UBossPoiseBrokenComponent::UnlockRotation()
{
	AWGasCharacterEnemy* Enemy = OwnerEnemy.Get();
	if (!Enemy)return;
	bPoiseBrokenRotationLocked = false;
	if (bRotationLocked)
	{
		Enemy->bUseControllerRotationYaw = bCachedUseControllerRotationYaw;
		bRotationLocked = false;
	}
	if (UCharacterMovementComponent* Movement = Enemy->GetCharacterMovement())
	{
		Movement->bUseControllerDesiredRotation = bCachedUseControllerDesiredRotation;
		Movement->bOrientRotationToMovement = bCachedOrientRotationToMovement;
		Movement->RotationRate = CachedRotationRate;
	}
}

bool UBossPoiseBrokenComponent::ShouldSkipPoise() const
{
	if (const UBossPhaseTransitionComponent* PhaseComp=Phase.Get())
	{
		if (PhaseComp->IsTransitionActive())return true;
	}
	AWGasCharacterEnemy* Enemy = OwnerEnemy.Get();
	if (!Enemy)return true;
	UWGasAbilitySystemComponent* ASC = Cast<UWGasAbilitySystemComponent>(Enemy->GetAbilitySystemComponent());
	if (!ASC)return true;
	const FWGasGameplayTags& WGasTags = FWGasGameplayTags::Get();
	if (WGasTags.State_Boss_PhaseTransition.IsValid()&& ASC->HasMatchingGameplayTag(WGasTags.State_Boss_PhaseTransition))
	{
		return true;
	}
	if (WGasTags.State_Boss_PoiseBroken.IsValid()&& ASC->HasMatchingGameplayTag(WGasTags.State_Boss_PoiseBroken))
	{
		return true;
	}
	return false;
}

void UBossPoiseBrokenComponent::OnPoiseBrokenMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != PoiseBrokenMontage || bInterrupted)
	{
		return;
	}
	bExitPending = false;
	ExitPoiseBroken();
}

