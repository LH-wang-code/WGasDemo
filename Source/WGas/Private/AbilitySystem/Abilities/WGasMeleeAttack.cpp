// Fill out your copyright notice in the Description page of Project Settings.



#include "AbilitySystem/Abilities/WGasMeleeAttack.h"



#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

#include "Animation/AnimInstance.h"

#include "Animation/AnimMontage.h"

#include "Character/WGasCharacterBase.h"

#include "Components/SkeletalMeshComponent.h"

#include "DrawDebugHelpers.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "KismetTraceUtils.h"
#include "MotionWarpingComponent.h"
#include "WGas.h"
#include "WGasGameplayTags.h"
#include "Character/WGasCharacterHero.h"
#include "Character/WGasLockOnComponent.h"
#include "Input/WGasInputComponent.h"


UWGasMeleeAttack::UWGasMeleeAttack()

{

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	const FWGasGameplayTags& WGasTags = FWGasGameplayTags::Get();
	AttackingActiveTag = WGasTags.State_Attacking_Active;
	AttackingRecoveryTag = WGasTags.State_Attacking_Recovery;
}

void UWGasMeleeAttack::InputPressed(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);
	if (IsInAttackRecovery())
	{
		if (TryCancelFromRecovery())
		{
			// 接段前强制刷新 Warp，不依赖蓝图是否再调 BeginMeleeAttack
			UpdateLockOnWarpTarget();
			OnRecoveryCancelIntoCombo();
		}
		return;
	}
	if (AWGasCharacterHero* Character=Cast<AWGasCharacterHero>(GetWGasCharacterFromActorInfo()))
	{
		if (UGasInputBufferComponent*Buffer=Character->GetWGasInputBufferComponent())
		{
			//只有左键激活这个能力，检测到输入时我们直接放进去tag就行
			Buffer->BufferInput(FWGasGameplayTags::Get().InputTag_LMB);
		}
	}
	OnComboInputReceived();
}

void UWGasMeleeAttack::InputReleased(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
}


void UWGasMeleeAttack::BeginMeleeAttack()

{
	UpdateLockOnWarpTarget();
	EnterAttackActive();
	if (!bStopMovementOnAttack)
	{
		return;
	}
	if (AWGasCharacterBase* Character = GetWGasCharacterFromActorInfo())
	{
		if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
		}

		RegisterDamageWindow();
		
	}
}

bool UWGasMeleeAttack::UpdateLockOnWarpTarget()
{
	AWGasCharacterHero* Hero = Cast<AWGasCharacterHero>(GetWGasCharacterFromActorInfo());
	if (!Hero)
	{
		return false;
	}

	UWGasLockOnComponent* LockComp = Hero->GetLockOnComponent();
	if (!LockComp || !LockComp->IsLockedOn() || !LockComp->GetLockTarget())
	{
		return false;
	}

	const FVector LockPoint = LockComp->GetCurrentLockOnLocation();
	FVector ToTarget = LockPoint - Hero->GetActorLocation();
	ToTarget.Z = 0.f;

	FRotator FaceRot = Hero->GetActorRotation();
	if (!ToTarget.IsNearlyZero())
	{
		ToTarget.Normalize();
		FaceRot = FRotator(0.f, ToTarget.Rotation().Yaw, 0.f);
	}

	const float CurrentDist = FVector::Dist2D(Hero->GetActorLocation(), LockPoint);
	FVector WarpLoc = Hero->GetActorLocation();
	if (CurrentDist > LockOnAttackDistance + 20.f)
	{
		WarpLoc = LockPoint - ToTarget * LockOnAttackDistance;
	}
	WarpLoc.Z = Hero->GetActorLocation().Z;

	UMotionWarpingComponent* MWC = Hero->GetMotionWarpingComponent();
	if (!MWC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LockOnWarp] 无 MotionWarpingComponent"));
		return false;
	}

	MWC->AddOrUpdateWarpTargetFromLocationAndRotation(LockOnWarpTargetName, WarpLoc, FaceRot);
	Hero->SetActorRotation(FaceRot);

	if (UCharacterMovementComponent* Move = Hero->GetCharacterMovement())
	{
		Move->bAllowPhysicsRotationDuringAnimRootMotion = true;
	}

#if !UE_BUILD_SHIPPING
	UE_LOG(LogTemp, Log, TEXT("[LockOnWarp] ComboIdx=%d Dist=%.0f WarpLoc=%s"),
		ComboIdx, CurrentDist, *WarpLoc.ToString());
	DrawDebugSphere(Hero->GetWorld(), WarpLoc, 30.f, 12, FColor::Yellow, false, 1.5f);
#endif

	return true;
}

void UWGasMeleeAttack::ClearLockOnWarpTarget() const
{
	if (const AWGasCharacterBase* Character = GetWGasCharacterFromActorInfo())
	{
		if (UMotionWarpingComponent* MWC = Character->FindComponentByClass<UMotionWarpingComponent>())
		{
			MWC->RemoveWarpTarget(LockOnWarpTargetName);
		}
	}
}

void UWGasMeleeAttack::EndMeleeAttack(bool bWasCancelled)
{
	RemoveAttackingTags();
	UnregisterDamageWindow();  
	if (bWasCancelled)
	{
		K2_CancelAbility();
	}
	else
	{
		K2_EndAbility();
	}
	UnregisterDamageWindow();

	// 接段取消时不删 Warp，下一刀还要用；整段结束再清
	if (!bWasCancelled)
	{
		ClearLockOnWarpTarget();
	}
}

void UWGasMeleeAttack::EnterAttackReovery()
{
	UAbilitySystemComponent* ASC = GetWGasASCFromActorInfo();
	if (!ASC) return;
	const FWGasGameplayTags& Tags = FWGasGameplayTags::Get();
	if (Tags.State_Attacking_Active.IsValid())
	{
		ASC->RemoveLooseGameplayTag(Tags.State_Attacking_Active);
	}
	if (Tags.State_Attacking_Recovery.IsValid())
	{
		ASC->AddLooseGameplayTag(Tags.State_Attacking_Recovery);
	}
	UnregisterDamageWindow();
}

bool UWGasMeleeAttack::IsInAttackActive() const
{
	const UAbilitySystemComponent* ASC = GetWGasASCFromActorInfo();
	if (!ASC) return false;
	return ASC->HasMatchingGameplayTag(FWGasGameplayTags::Get().State_Attacking_Active);
}

bool UWGasMeleeAttack::IsInAttackRecovery() const
{
	const UAbilitySystemComponent* ASC = GetWGasASCFromActorInfo();
	if (!ASC) return false;
	return ASC->HasMatchingGameplayTag(FWGasGameplayTags::Get().State_Attacking_Recovery);
}

bool UWGasMeleeAttack::TryCancelFromRecovery()
{
	if (!IsInAttackRecovery())return false;
	StopCurrentAttackMontage(0.1f);
	EndMeleeAttack(true);  // true = K2_CancelAbility
	return true;
}

void UWGasMeleeAttack::OnMeleeMontageFinished(bool bWasCancelled)
{
	EndMeleeAttack(bWasCancelled);
}
void UWGasMeleeAttack::RemoveAttackingTags() const
{
	if (!AttackingStateTag.IsValid())
	{
		return;
	}
	if (UAbilitySystemComponent* ASC = GetWGasASCFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(AttackingStateTag);
	}
}

void UWGasMeleeAttack::EnterAttackActive()
{
	//进入攻击阶段，这时不可以被打断
	UAbilitySystemComponent* ASC = GetWGasASCFromActorInfo();
	if (!ASC) return;
	const FWGasGameplayTags& WGasTags = FWGasGameplayTags::Get();
	if (AttackingStateTag.IsValid())
	{
		ASC->AddLooseGameplayTag(AttackingStateTag);
	}
	
	if (WGasTags.State_Attacking_Active.IsValid())
	{
		ASC->AddLooseGameplayTag(WGasTags.State_Attacking_Active);
	}
	if (WGasTags.State_Attacking_Recovery.IsValid())
	{
		ASC->RemoveLooseGameplayTag(WGasTags.State_Attacking_Recovery);
	}
}

void UWGasMeleeAttack::RemoveAllAttackingTags() const
{
	UAbilitySystemComponent* ASC = GetWGasASCFromActorInfo();
	if (!ASC) return;
	const FWGasGameplayTags& WGasTags = FWGasGameplayTags::Get();
	if (AttackingStateTag.IsValid())
	{
		ASC->RemoveLooseGameplayTag(AttackingStateTag);
	}
	if (WGasTags.State_Attacking_Active.IsValid())
	{
		ASC->RemoveLooseGameplayTag(WGasTags.State_Attacking_Active);
	}
	if (WGasTags.State_Attacking_Recovery.IsValid())
	{
		ASC->RemoveLooseGameplayTag(WGasTags.State_Attacking_Recovery);
	}
}

void UWGasMeleeAttack::StopCurrentAttackMontage(float BlendOutTime) const
{
	if (const AWGasCharacterBase* Character = GetWGasCharacterFromActorInfo())
	{
		if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			if (UAnimInstance* Anim = Mesh->GetAnimInstance())
			{
				Anim->Montage_Stop(BlendOutTime);
			}
		}
	}
}

