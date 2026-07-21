// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/WGasMomentumUltimate.h"

#include "AbilitySystemBlueprintLibrary.h"
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
	SegmentPoiseDamages = { 65.f, 0.f, 0.f };
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

FDamagePayLoad UWGasMomentumUltimate::BuildSegmentDamagePayload(const int32 SegmentIndex) const
{
	FDamagePayLoad Payload = BuildDamagePayload();
	Payload.BasePoiseDamage = 0.f;

	if (SegmentDamages.IsValidIndex(SegmentIndex) && SegmentDamages[SegmentIndex] > 0.f)
	{
		Payload.BaseDamage = SegmentDamages[SegmentIndex];
	}

	if (SegmentPoiseDamages.IsValidIndex(SegmentIndex))
	{
		Payload.BasePoiseDamage = FMath::Max(0.f, SegmentPoiseDamages[SegmentIndex]);
	}

	return Payload;
}

void UWGasMomentumUltimate::RegisterSegmentDamageWindow(const int32 SegmentIndex)
{
	AWGasCharacterBase* Character = GetWGasCharacterFromActorInfo();
	if (!Character || !Character->CombatComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("BeginUltimateSegment: 缺少 Character 或 CombatComponent"));
		return;
	}

	const FDamagePayLoad Payload = BuildSegmentDamagePayload(SegmentIndex);
	if (!Payload.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("BeginUltimateSegment(%d): Payload 无效"), SegmentIndex);
		return;
	}

#if !UE_BUILD_SHIPPING
	UE_LOG(LogTemp, Warning, TEXT("[MomentumUltimate] Segment=%d  Damage=%.1f  PoiseDamage=%.1f"),
		SegmentIndex, Payload.BaseDamage, Payload.BasePoiseDamage);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
			FString::Printf(TEXT("Ultimate Seg %d | Dmg %.0f | Poise %.0f"),
				SegmentIndex, Payload.BaseDamage, Payload.BasePoiseDamage));
	}
#endif

	Character->CombatComponent->BeginDamageWindow(Payload);
}

void UWGasMomentumUltimate::BeginUltimateSegment(const int32 SegmentIndex)
{
	CurrentSegmentIndex = SegmentIndex;
	RegisterSegmentDamageWindow(SegmentIndex);
}

void UWGasMomentumUltimate::BeginMomentumUltimate()
{
	bUltimateEndHandled = false;
	CurrentSegmentIndex = -1;

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

void UWGasMomentumUltimate::ApplyGameplayEffectToTarget(
	UAbilitySystemComponent* TargetASC,
	const TSubclassOf<UGameplayEffect> EffectClass) const
{
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!EffectClass || !SourceASC || !TargetASC)
	{
		return;
	}

	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		Context.AddSourceObject(Avatar);
	}

	const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(EffectClass, 1.f, Context);
	if (!SpecHandle.IsValid())
	{
		return;
	}

	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
}

bool UWGasMomentumUltimate::HasAllMomentumMarks(const UAbilitySystemComponent* TargetASC) const
{
	if (!TargetASC)
	{
		return false;
	}

	const FWGasGameplayTags& Tags = FWGasGameplayTags::Get();
	return Tags.Mark_MomentumULtimate_Layer_1.IsValid()
		&& Tags.Mark_MomentumULtimate_Layer_2.IsValid()
		&& Tags.Mark_MomentumULtimate_Layer_3.IsValid()
		&& TargetASC->HasMatchingGameplayTag(Tags.Mark_MomentumULtimate_Layer_1)
		&& TargetASC->HasMatchingGameplayTag(Tags.Mark_MomentumULtimate_Layer_2)
		&& TargetASC->HasMatchingGameplayTag(Tags.Mark_MomentumULtimate_Layer_3);
}

void UWGasMomentumUltimate::ClearMomentumMarksAndDebuffs(UAbilitySystemComponent* TargetASC) const
{
	if (!TargetASC)
	{
		return;
	}

	const FWGasGameplayTags& Tags = FWGasGameplayTags::Get();
	FGameplayTagContainer RemoveTags;
	if (Tags.Mark_MomentumULtimate_Layer_1.IsValid()) { RemoveTags.AddTag(Tags.Mark_MomentumULtimate_Layer_1); }
	if (Tags.Mark_MomentumULtimate_Layer_2.IsValid()) { RemoveTags.AddTag(Tags.Mark_MomentumULtimate_Layer_2); }
	if (Tags.Mark_MomentumULtimate_Layer_3.IsValid()) { RemoveTags.AddTag(Tags.Mark_MomentumULtimate_Layer_3); }
	if (Tags.Debuff_MomentumUltimate_Bleed.IsValid()) { RemoveTags.AddTag(Tags.Debuff_MomentumUltimate_Bleed); }
	if (Tags.Debuff_MomentumUltimate_Vulnerable.IsValid()) { RemoveTags.AddTag(Tags.Debuff_MomentumUltimate_Vulnerable); }

	if (!RemoveTags.IsEmpty())
	{
		TargetASC->RemoveActiveEffectsWithGrantedTags(RemoveTags);
	}
}

void UWGasMomentumUltimate::TryDetonateMomentumMarks(UAbilitySystemComponent* TargetASC)
{
	if (!HasAllMomentumMarks(TargetASC))
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC || !DamageEffectClass || DetonationMaxHealthPercent <= 0.f)
	{
		ClearMomentumMarksAndDebuffs(TargetASC);
		return;
	}

	const UWGasAttributeSet* TargetAS = Cast<UWGasAttributeSet>(
		TargetASC->GetAttributeSet(UWGasAttributeSet::StaticClass()));
	if (!TargetAS)
	{
		return;
	}

	const float DetonationDamage = TargetAS->GetMaxHealth() * DetonationMaxHealthPercent;
	if (DetonationDamage > 0.f)
	{
		FDamageEffectParams Params;
		Params.DamageGameplayEffectClass = DamageEffectClass;
		Params.SourceAbilitySystemComponent = SourceASC;
		Params.TargetAbilitySystemComponent = TargetASC;
		Params.DamageType = DamageType;
		Params.BaseDamage = DetonationDamage;
		Params.BasePoiseDamage = 0.f;
		UWGasAbilitySystemFunctionLibrary::ApplyDamageEffectParams(Params);
	}

	ClearMomentumMarksAndDebuffs(TargetASC);
}

void UWGasMomentumUltimate::ApplyMomentumLayerToTarget(const int32 SegmentIndex, AActor* TargetActor)
{
	if (!TargetActor || SegmentIndex < 0 || SegmentIndex > 2)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC)
	{
		return;
	}

	if (MarkEffectClasses.IsValidIndex(SegmentIndex) && MarkEffectClasses[SegmentIndex])
	{
		ApplyGameplayEffectToTarget(TargetASC, MarkEffectClasses[SegmentIndex]);
	}

	switch (SegmentIndex)
	{
	case 1:
		if (BleedEffectClass)
		{
			ApplyGameplayEffectToTarget(TargetASC, BleedEffectClass);
		}
		break;
	case 2:
		if (VulnerableEffectClass)
		{
			ApplyGameplayEffectToTarget(TargetASC, VulnerableEffectClass);
		}
		TryDetonateMomentumMarks(TargetASC);
		break;
	default:
		break;
	}
}
