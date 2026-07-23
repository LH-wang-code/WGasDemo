// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Boss/WGasBossMeleeAttack.h"

#include "MotionWarpingComponent.h"
#include "AIController.h"
#include "WGasGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/WGasCharacterBase.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interaction/BossCombatInterface.h"

UWGasBossMeleeAttack::UWGasBossMeleeAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	const FWGasGameplayTags& Tags = FWGasGameplayTags::Get();
	FGameplayTagContainer AssetTagContainer;
	if (Tags.Abilities_Attack_Melee.IsValid())
	{
		AbilityTags.AddTag(Tags.Abilities_Attack_Melee);
		AssetTagContainer.AddTag(Tags.Abilities_Attack_Melee);
	}
	SetAssetTags(AssetTagContainer);
	if (Tags.State_Boss_Attacking.IsValid())
	{
		ActivationBlockedTags.AddTag(Tags.State_Boss_Attacking);
	}
	if (Tags.State_Boss_PoiseBroken.IsValid())
	{
		ActivationBlockedTags.AddTag(Tags.State_Boss_PoiseBroken);
	}
	if (Tags.State_Boss_PhaseTransition.IsValid())
	{
		ActivationBlockedTags.AddTag(Tags.State_Boss_PhaseTransition);
	}
	if (Tags.State_Boss_Invulnerable.IsValid())
	{
		ActivationBlockedTags.AddTag(Tags.State_Boss_Invulnerable);
	}
	if (Tags.State_Parried.IsValid())
	{
		ActivationBlockedTags.AddTag(Tags.State_Parried);
	}
}

void UWGasBossMeleeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	if (!AttackMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	BeginBossMeleeAttack();

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, AttackMontage);
	MontageTask->OnCompleted.AddDynamic(this, &UWGasBossMeleeAttack::OnAttackMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &UWGasBossMeleeAttack::OnAttackMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &UWGasBossMeleeAttack::OnAttackMontageEnded);
	MontageTask->ReadyForActivation();
}

void UWGasBossMeleeAttack::BeginBossMeleeAttack()
{
	bAttackEndHandled = false;
	if (bFaceTargetOnAttack)
	{
		FaceTargetActor();
	}
	ApplyAttackingTags();
	if (!bStopMovementOnAttack)
	{
		return;
	}
	if (AWGasCharacterBase* Character = GetWGasCharacterFromActorInfo())
	{
		if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
		{
			bCachedOrientRotationToMovement = Movement->bOrientRotationToMovement;
			bCachedUseControllerDesiredRotation = Movement->bUseControllerDesiredRotation;
			bCachedAllowPhysicsRotationDuringAnimRootMotion = Movement->bAllowPhysicsRotationDuringAnimRootMotion;
			Movement->bOrientRotationToMovement = false;
			Movement->bUseControllerDesiredRotation = false;
			Movement->bAllowPhysicsRotationDuringAnimRootMotion = true;
			Movement->StopMovementImmediately();
		}
		RegisterDamageWindow();
	}
}

void UWGasBossMeleeAttack::FaceTargetActor()
{
	AWGasCharacterBase* Character = GetWGasCharacterFromActorInfo();
	if (!Character)
	{
		return;
	}

	AAIController* AIController = Cast<AAIController>(Character->GetController());

	AActor* Target = nullptr;
	if (AIController)
	{
		if (const UBlackboardComponent* BB = AIController->GetBlackboardComponent())
		{
			Target = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));
		}
	}
	if (!Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BossWarp] TargetActor 未设置，跳过朝向与 Warp"));
		return;
	}

	FVector TargetLoc = Target->GetActorLocation();
	if (const ACharacter* TargetChar = Cast<ACharacter>(Target))
	{
		TargetLoc.Z += TargetChar->GetSimpleCollisionHalfHeight();
	}

	FVector ToTarget = Target->GetActorLocation() - Character->GetActorLocation();
	ToTarget.Z = 0.f;

	FRotator FaceRot;
	if (ToTarget.Normalize())
	{
		FaceRot = FRotator(0.f, ToTarget.Rotation().Yaw, 0.f);
	}
	else if (AIController)
	{
		// 贴脸时水平方向几乎为 0，用 AI 当前朝向（MoveTo 通常已对准玩家）
		FaceRot = FRotator(0.f, AIController->GetControlRotation().Yaw, 0.f);
	}
	else
	{
		FaceRot = FRotator(0.f, Character->GetActorRotation().Yaw, 0.f);
	}

	UMotionWarpingComponent* MWC = Character->FindComponentByClass<UMotionWarpingComponent>();

	if (MWC)
	{
		MWC->AddOrUpdateWarpTargetFromLocationAndRotation(FName("AttackTarget"), TargetLoc, FaceRot);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[BossWarp] 未找到 MotionWarpingComponent"));
	}

#if !UE_BUILD_SHIPPING
	if (AttackMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BossWarp] Montage=%s EnableRootMotion=%d"),
			*AttackMontage->GetName(), AttackMontage->HasRootMotion());
	}
#endif

	Character->SetActorRotation(FaceRot);
	if (AIController)
	{
		AIController->ClearFocus(EAIFocusPriority::Gameplay);
		AIController->SetControlRotation(FaceRot);
	}

#if !UE_BUILD_SHIPPING
	if (UWorld* World = Character->GetWorld())
	{
		DrawDebugSphere(World, TargetLoc, 35.f, 12, FColor::Yellow, false, 2.f);
		DrawDebugLine(World, Character->GetActorLocation(), TargetLoc, FColor::Cyan, false, 2.f, 0, 2.f);
	}
#endif
}

void UWGasBossMeleeAttack::OnAttackMontageEnded()
{
	FinishAttack(false);
}

void UWGasBossMeleeAttack::EndBossMeleeAttack(bool bWasCancelled)
{
	FinishAttack(bWasCancelled);
}

UWGasBossMeleeAttack* UWGasBossMeleeAttack::GetActiveBossMeleeAttack(UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		return nullptr;
	}
	for (const FGameplayAbilitySpec& Spec:ASC->GetActivatableAbilities())
	{
		if (!Spec.IsActive())
		{
			continue;
		}
		for (UGameplayAbility* Instance : Spec.GetAbilityInstances())
		{
			if (UWGasBossMeleeAttack* Melee = Cast<UWGasBossMeleeAttack>(Instance))
			{
				return Melee;
			}
		}
	}
	return nullptr;
}

bool UWGasBossMeleeAttack::TryInterruptFromParry(UAbilitySystemComponent* AttackerASC)
{
	if (UWGasBossMeleeAttack* BossMelee = GetActiveBossMeleeAttack(AttackerASC))
		return BossMelee->TryInterruptFromParry();
	return false;
}

bool UWGasBossMeleeAttack::TryInterruptFromParry()
{
	if (bAttackEndHandled || !bInterruptibleByParry)
		return false;
	if (AWGasCharacterBase* Character = GetWGasCharacterFromActorInfo())
	{
		if (Character->CombatComponent)
			Character->CombatComponent->EndWeaponSweep();
	}
	StopAttackMontage(0.1f);
	FinishAttack(true);  
	return true;
}

void UWGasBossMeleeAttack::StopAttackMontage(float BlendOutTime) const
{
	if (!AttackMontage)
	{
		return;
	}

	if (const AWGasCharacterBase* Character = GetWGasCharacterFromActorInfo())
	{
		if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			if (UAnimInstance* Anim = Mesh->GetAnimInstance())
			{
				Anim->Montage_Stop(BlendOutTime, AttackMontage);
			}
		}
	}
}

void UWGasBossMeleeAttack::FinishAttack(bool bWasCancelled)
{
	if (bAttackEndHandled)
	{
		return;
	}
	bAttackEndHandled = true;
	if (IBossCombatInterface* BossCombat = Cast<IBossCombatInterface>(GetAvatarActorFromActorInfo()))
	{
		BossCombat->NotifyBossNormalAttackFinished();
	}
	if (AWGasCharacterBase* Character = GetWGasCharacterFromActorInfo())
	{
		if (Character->CombatComponent)
		{
			Character->CombatComponent->EndDamageWindow();
		}
		if (UMotionWarpingComponent* MWC = Character->FindComponentByClass<UMotionWarpingComponent>())
		{
			MWC->RemoveWarpTarget(FName("AttackTarget"));
		}
		if (AAIController* AIController = Cast<AAIController>(Character->GetController()))
		{
			AIController->ClearFocus(EAIFocusPriority::Gameplay);
		}
		if (bStopMovementOnAttack)
		{
			if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
			{
				Movement->bOrientRotationToMovement = bCachedOrientRotationToMovement;
				Movement->bUseControllerDesiredRotation = bCachedUseControllerDesiredRotation;
				Movement->bAllowPhysicsRotationDuringAnimRootMotion = bCachedAllowPhysicsRotationDuringAnimRootMotion;
			}
		}
	}
	UnregisterDamageWindow();
	RemoveAttackingTags();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bWasCancelled);
}

void UWGasBossMeleeAttack::ApplyAttackingTags() const
{
	const FGameplayTag& Tag = FWGasGameplayTags::Get().State_Boss_Attacking;
	if (!Tag.IsValid())
	{
		return;
	}
	if (UAbilitySystemComponent* ASC = GetWGasASCFromActorInfo())
	{
		ASC->AddLooseGameplayTag(Tag);
	}
}

void UWGasBossMeleeAttack::RemoveAttackingTags() const
{
	const FGameplayTag& Tag = FWGasGameplayTags::Get().State_Boss_Attacking;
	if (!Tag.IsValid())
	{
		return;
	}
	if (UAbilitySystemComponent* ASC = GetWGasASCFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(Tag);
	}
}
