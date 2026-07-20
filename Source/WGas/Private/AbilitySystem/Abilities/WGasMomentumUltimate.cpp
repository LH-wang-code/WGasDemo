// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/WGasMomentumUltimate.h"

#include "WGasGameplayTags.h"
#include "AbilitySystem/WGasAbilitySystemFunctionLibrary.h"
#include "AbilitySystem/WGasAttributeSet.h"
#include "AbilitySystem/Abilities/WGasMeleeAttack.h"
#include "Character/WGasCharacterBase.h"
#include "Character/WGasCharacterHero.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MotionWarpingComponent.h"
#include "DrawDebugHelpers.h"

UWGasMomentumUltimate::UWGasMomentumUltimate()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UWGasMomentumUltimate::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const UWGasAttributeSet* AS = Cast<UWGasAttributeSet>(
		ActorInfo->AbilitySystemComponent->GetAttributeSet(UWGasAttributeSet::StaticClass()));
	if (!AS)
	{
		return false;
	}

	return AS->GetMaxMomentum() > 0.f && AS->GetMomentum() >= AS->GetMaxMomentum();
}

void UWGasMomentumUltimate::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		if (UWGasMeleeAttack* Melee = UWGasMeleeAttack::GetActiveMeleeAttack(ASC))
		{
			Melee->TryCancelFromCancelablePhase();
		}
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (UAbilitySystemComponent* ASC = GetWGasASCFromActorInfo())
	{
		UWGasAbilitySystemFunctionLibrary::ConsumeAllMomentum(ASC);

		const FWGasGameplayTags& WGasTags = FWGasGameplayTags::Get();
		if (WGasTags.State_Momentum_Full.IsValid())
		{
			ASC->RemoveLooseGameplayTag(WGasTags.State_Momentum_Full);
		}
	}

	BeginMomentumUltimate();
	OnMomentumUltimateActivated();
}

void UWGasMomentumUltimate::BeginMomentumUltimate()
{
	bUltimateEndHandled = false;

	if (UAbilitySystemComponent* ASC = GetWGasASCFromActorInfo())
	{
		const FGameplayTag& ActiveTag = FWGasGameplayTags::Get().State_Attacking_Active;
		if (ActiveTag.IsValid())
		{
			ASC->AddLooseGameplayTag(ActiveTag);
		}
	}

	if (AWGasCharacterBase* Character = GetWGasCharacterFromActorInfo())
	{
		if (bStopMovementOnAttack)
		{
			if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
			{
				Movement->StopMovementImmediately();
			}
		}
	}

	RegisterDamageWindow();
}

void UWGasMomentumUltimate::EndMomentumUltimate(const bool bWasCancelled)
{
	FinishUltimate(bWasCancelled);
}

bool UWGasMomentumUltimate::IsMomentumUltimateLockOnActive() const
{
	const AWGasCharacterHero* Hero = Cast<AWGasCharacterHero>(GetAvatarActorFromActorInfo());
	return Hero && Hero->IsLockOnActive();
}

AActor* UWGasMomentumUltimate::GetMomentumUltimateLockTarget() const
{
	const AWGasCharacterHero* Hero = Cast<AWGasCharacterHero>(GetAvatarActorFromActorInfo());
	return Hero ? Hero->GetLockOnTargetActor() : nullptr;
}

FVector UWGasMomentumUltimate::GetMomentumUltimateLockOnLocation() const
{
	const AWGasCharacterHero* Hero = Cast<AWGasCharacterHero>(GetAvatarActorFromActorInfo());
	return Hero ? Hero->GetLockOnTargetLocation() : FVector::ZeroVector;
}

bool UWGasMomentumUltimate::UpdateUltimateWarpTarget()
{
	AWGasCharacterHero* Hero = Cast<AWGasCharacterHero>(GetAvatarActorFromActorInfo());
	if (!Hero)
	{
		return false;
	}

	UMotionWarpingComponent* MWC = Hero->GetMotionWarpingComponent();
	if (!MWC)
	{
		return false;
	}

	const FVector SelfLoc = Hero->GetActorLocation();
	FVector ToTarget = FVector::ZeroVector;
	FVector LockPoint = FVector::ZeroVector;
	bool bHasLockPoint = false;

	if (Hero->IsLockOnActive() && Hero->GetLockOnTargetActor())
	{
		LockPoint = Hero->GetLockOnTargetLocation();
		if (!LockPoint.IsNearlyZero())
		{
			ToTarget = LockPoint - SelfLoc;
			ToTarget.Z = 0.f;
			bHasLockPoint = !ToTarget.IsNearlyZero();
		}
	}

	FRotator FaceRot = Hero->GetActorRotation();
	if (bHasLockPoint)
	{
		ToTarget.Normalize();
		FaceRot = FRotator(0.f, ToTarget.Rotation().Yaw, 0.f);
	}
	else
	{
		ToTarget = Hero->GetActorForwardVector();
		ToTarget.Z = 0.f;
		if (!ToTarget.IsNearlyZero())
		{
			ToTarget.Normalize();
			FaceRot = FRotator(0.f, ToTarget.Rotation().Yaw, 0.f);
		}
		LockPoint = SelfLoc + ToTarget * StrikeStopDistance;
		bHasLockPoint = true;
	}

	if (!bHasLockPoint)
	{
		return false;
	}

	FVector WarpLoc = SelfLoc;
	if (bUsePierceWarpTarget)
	{
		WarpLoc = LockPoint + ToTarget * PiercePastDistance;
	}
	else
	{
		const float CurrentDist = FVector::Dist2D(SelfLoc, LockPoint);
		if (CurrentDist > StrikeStopDistance + 20.f)
		{
			WarpLoc = LockPoint - ToTarget * StrikeStopDistance;
		}
	}
	WarpLoc.Z = SelfLoc.Z;

	MWC->AddOrUpdateWarpTargetFromLocationAndRotation(LockOnWarpTargetName, WarpLoc, FaceRot);
	Hero->SetActorRotation(FaceRot);

	if (UCharacterMovementComponent* Move = Hero->GetCharacterMovement())
	{
		Move->bAllowPhysicsRotationDuringAnimRootMotion = true;
	}

#if !UE_BUILD_SHIPPING
	DrawDebugSphere(Hero->GetWorld(), WarpLoc, 30.f, 12, FColor::Orange, false, 1.5f);
	DrawDebugLine(Hero->GetWorld(), SelfLoc, WarpLoc, FColor::Orange, false, 1.5f, 0, 2.f);
#endif

	return true;
}

void UWGasMomentumUltimate::ClearUltimateWarpTarget()
{
	if (const AWGasCharacterHero* Hero = Cast<AWGasCharacterHero>(GetAvatarActorFromActorInfo()))
	{
		if (UMotionWarpingComponent* MWC = Hero->GetMotionWarpingComponent())
		{
			MWC->RemoveWarpTarget(LockOnWarpTargetName);
		}
	}
}

void UWGasMomentumUltimate::FinishUltimate(const bool bWasCancelled)
{
	if (bUltimateEndHandled)
	{
		return;
	}
	bUltimateEndHandled = true;

	ClearUltimateWarpTarget();

	if (UAbilitySystemComponent* ASC = GetWGasASCFromActorInfo())
	{
		const FWGasGameplayTags& Tags = FWGasGameplayTags::Get();
		if (Tags.State_Attacking_Active.IsValid())
		{
			ASC->RemoveLooseGameplayTag(Tags.State_Attacking_Active);
		}
	}

	UnregisterDamageWindow();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bWasCancelled);
}

void UWGasMomentumUltimate::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility,
	const bool bWasCancelled)
{
	if (!bUltimateEndHandled)
	{
		ClearUltimateWarpTarget();

		if (UAbilitySystemComponent* ASC = GetWGasASCFromActorInfo())
		{
			const FGameplayTag& ActiveTag = FWGasGameplayTags::Get().State_Attacking_Active;
			if (ActiveTag.IsValid())
			{
				ASC->RemoveLooseGameplayTag(ActiveTag);
			}
		}
		UnregisterDamageWindow();
		bUltimateEndHandled = true;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
