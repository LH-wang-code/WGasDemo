// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/WGasCombatComponent.h"

#include "AbilitySystem/WGasAbilitySystemFunctionLibrary.h"
#include "Character/WGasCharacterBase.h"

UWGasCombatComponent::UWGasCombatComponent()
{

	PrimaryComponentTick.bCanEverTick = true;

}

void UWGasCombatComponent::BeginDamageWindow(const FDamagePayLoad& Payload)
{

	CurrentPayload = Payload;
	bDamageWindowActive = true;
	HitActorsThisSwing.Empty();
}

void UWGasCombatComponent::EndDamageWindow()
{
	bWeaponSweepActive = false;
	bDamageWindowActive = false;
	CurrentPayload = FDamagePayLoad{};
	HitActorsThisSwing.Empty();
}

void UWGasCombatComponent::BeginWeaponSweep()
{
	if (!bDamageWindowActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] BeginWeaponSweep: 未开 DamageWindow，先让 GA 调 BeginDamageWindow"),
			*GetNameSafe(GetOwner()));
		return;
	}

	USkeletalMeshComponent* TraceMesh = GetTraceMesh();
	if (TraceMesh
		&& TraceMesh->DoesSocketExist(WeaponBaseSocket)
		&& TraceMesh->DoesSocketExist(WeaponTipSocket))
	{
		LastWeaponBasePos = TraceMesh->GetSocketLocation(WeaponBaseSocket);
		LastWeaponTipPos  = TraceMesh->GetSocketLocation(WeaponTipSocket);
	}
	HitActorsThisSwing.Empty();
	bWeaponSweepActive = true;

}

void UWGasCombatComponent::EndWeaponSweep()
{
	bWeaponSweepActive = false;
	HitActorsThisSwing.Empty();
}

void UWGasCombatComponent::BeginPlay()
{
	Super::BeginPlay();


	
}

void UWGasCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bWeaponSweepActive)
	{
		TickWeaponSweep();
	}
}

void UWGasCombatComponent::TickWeaponSweep()
{
	if (!bWeaponSweepActive)
	{
		return;
	}
	USkeletalMeshComponent* TraceMesh = GetTraceMesh();
	if (!TraceMesh|| !TraceMesh->DoesSocketExist(WeaponBaseSocket)|| !TraceMesh->DoesSocketExist(WeaponTipSocket))
	{
		#if !UE_BUILD_SHIPPING
				static TSet<TWeakObjectPtr<const UWGasCombatComponent>> Logged;
				if (!Logged.Contains(this))
				{
					Logged.Add(this);
					UE_LOG(LogTemp, Warning, TEXT("[%s] WeaponSweep 缺少 Socket: %s / %s"),
						*GetNameSafe(GetOwner()),
						*WeaponBaseSocket.ToString(),
						*WeaponBaseSocket.ToString());
				}
		#endif
		return;
	}

	UWorld* World = GetWorld();
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (!World || !OwnerCharacter)
	{
		return;
	}
	const int32 SampleCount = FMath::Max(WeaponSweepSampleCount, 1);
	const FVector CurrentBase = TraceMesh->GetSocketLocation(WeaponBaseSocket);
	const FVector CurrentTip  = TraceMesh->GetSocketLocation(WeaponTipSocket);

	
	FCollisionQueryParams Params(SCENE_QUERY_STAT(WeaponSweep), false, OwnerCharacter);
	Params.AddIgnoredActor(OwnerCharacter);

	
	for (int32 i = 0; i <= SampleCount; ++i)
	{
		const float Alpha = static_cast<float>(i) / static_cast<float>(SampleCount);
		const FVector LastPos    = FMath::Lerp(LastWeaponBasePos, LastWeaponTipPos, Alpha);
		const FVector CurrentPos = FMath::Lerp(CurrentBase, CurrentTip, Alpha);
		if (LastPos.Equals(CurrentPos, KINDA_SMALL_NUMBER))
		{
			continue;
		}
		FHitResult Hit;
		const bool bHit = World->SweepSingleByChannel(
			Hit,
			LastPos,
			CurrentPos,
			FQuat::Identity,
			ECC_Pawn,
			FCollisionShape::MakeSphere(WeaponSweepRadius),
			Params);
		if (bDrawDebug)
		{
			DrawDebugLine(World, LastPos, CurrentPos, FColor::Red, false, 0.5f, 0, 2.f);
			DrawDebugSphere(World, CurrentPos, WeaponSweepRadius, 8, FColor::Yellow, false, 0.5f);
		}
		if (bHit && Hit.GetActor())
		{
			NotifyHit(Hit.GetActor());
		}
	}
	LastWeaponBasePos = CurrentBase;
	LastWeaponTipPos  = CurrentTip;
}

USkeletalMeshComponent* UWGasCombatComponent::GetTraceMesh() const
{
	if (AWGasCharacterBase* Character=Cast<AWGasCharacterBase>(GetOwner()))
	{
		return Character->GetWeaponTraceMesh();
	}
	return nullptr;
}

ACharacter* UWGasCombatComponent::GetOwnerCharacter() const
{
	return Cast<ACharacter>(GetOwner());
}

void UWGasCombatComponent::NotifyHit(AActor* HitActor)
{
	if (!HitActor || HitActorsThisSwing.Contains(HitActor))
	{
		return;
	}
	ICombatInterface* Damageable = Cast<ICombatInterface>(HitActor);
	if (!Damageable || !Damageable->IsAliveForCombat())
	{
		return;
	}
	UAbilitySystemComponent* TargetASC = Damageable->GetDamageableASC();
	if (!TargetASC)
	{
		return;
	}
	if (!bDamageWindowActive || !CurrentPayload.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Hit %s 但 Payload 无效或未开 DamageWindow"),
			*GetNameSafe(GetOwner()), *HitActor->GetName());
		return;
	}

	FDamageEffectParams Params;
	Params.DamageGameplayEffectClass       = CurrentPayload.DamageEffectClass;
	Params.SourceAbilitySystemComponent    = CurrentPayload.SourceASC;
	Params.TargetAbilitySystemComponent    = TargetASC;
	Params.DamageType                      = CurrentPayload.DamageType;
	Params.BaseDamage                      = CurrentPayload.BaseDamage;
	Params.BasePoiseDamage				   = CurrentPayload.BasePoiseDamage;
	UWGasAbilitySystemFunctionLibrary::ApplyDamageEffectParams(Params);
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green,
			FString::Printf(TEXT("Hit: %s  Dmg: %.0f"), *HitActor->GetName(), CurrentPayload.BaseDamage));
	}
#endif
	HitActorsThisSwing.Add(HitActor);
}

