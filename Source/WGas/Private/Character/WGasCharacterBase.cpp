// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/WGasCharacterBase.h"

#include "WGasGameplayTags.h"
#include "AbilitySystem/WGasAbilitySystemComponent.h"
#include "AbilitySystem/WGasAttributeSet.h"
#include "AbilitySystem/Abilities/WGasDamageGameplayAbility.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/FastReferenceCollector.h"

// Sets default values
AWGasCharacterBase::AWGasCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UWGasAbilitySystemComponent>("AbilitySystemComponent");
	AttributeSet = CreateDefaultSubobject<UWGasAttributeSet>("AttributeSet");
	
	Weapon = CreateDefaultSubobject< USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(), FName("weapon_r"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->JumpZVelocity = 500.f;
	Movement->AirControl = 0.35f;
	Movement->MaxWalkSpeed = 600.f;
}

UAbilitySystemComponent* AWGasCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AWGasCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWGasCharacterBase::InitAbilityActorInfo()
{
	if (UWGasAbilitySystemComponent* WGasASC = Cast<UWGasAbilitySystemComponent>(AbilitySystemComponent))
	{
		WGasASC->InitAbilityActorInfo(this, this);
	}
}

void AWGasCharacterBase::AddCharacterAbilities()
{
	if (bStartupAbilitiesGiven || !AbilitySystemComponent)
	{
		return;
	}

	if (UWGasAbilitySystemComponent* WGasASC = Cast<UWGasAbilitySystemComponent>(AbilitySystemComponent))
	{
		WGasASC->AddCharacterAbilities(StartupAbilities);
		WGasASC->AddCharacterPassiveAbilities(StartupPassiveAbilities);
		
		bStartupAbilitiesGiven = true;
	}
}

void AWGasCharacterBase::InitializeDefaultAttributes() const
{
	ApplyEffectToSelf(DefaultSecondaryAttributes,1.f);
	ApplyEffectToSelf(DefaultVitalAttributes,1.f);

	if (const UWGasAttributeSet* AS = Cast<UWGasAttributeSet>(AttributeSet))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Init Attr | HP: %.1f/%.1f  Mana: %.1f/%.1f  Poise: %.1f/%.1f  Stamina: %.1f/%.1f"),
			*GetName(),
			AS->GetHealth(), AS->GetMaxHealth(),
			AS->GetMana(), AS->GetMaxMana(),
			AS->GetPoise(), AS->GetMaxPoise(),
			AS->GetStamina(), AS->GetMaxStamina());
	}
}

void AWGasCharacterBase::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const
{
	check(IsValid(GetAbilitySystemComponent()));
	check(GameplayEffectClass);
	FGameplayEffectContextHandle ContextHandle=GetAbilitySystemComponent()->MakeEffectContext();
	ContextHandle.AddSourceObject(this);
	
	const FGameplayEffectSpecHandle SpecHandle=GetAbilitySystemComponent()->MakeOutgoingSpec(GameplayEffectClass,Level,ContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
}

void AWGasCharacterBase::BeginWeaponSweep()
{
	if (USkeletalMeshComponent* TraceMesh=GetWeaponTraceMesh())
	{
		LastWeaponTipPos=TraceMesh->GetSocketLocation(WeaponTipSocket);
		LastWeaponBasePos=TraceMesh->GetSocketLocation(WeaponBaseSocket);
	}
	MeleeHitActorsThisSwing.Empty();
	bWeaponSweepActive = true;
}

void AWGasCharacterBase::TickWeaponSweep()
{
	if (!bWeaponSweepActive) return;
	USkeletalMeshComponent* TraceMesh = GetWeaponTraceMesh();
	if (!TraceMesh
	|| !TraceMesh->DoesSocketExist(WeaponBaseSocket)
	|| !TraceMesh->DoesSocketExist(WeaponTipSocket))
	{
		return;
	}
	const int32 SampleCount = FMath::Max(WeaponSweepSampleCount, 1);
	const FVector CurrentBase = TraceMesh->GetSocketLocation(WeaponBaseSocket);
	const FVector CurrentTip  = TraceMesh->GetSocketLocation(WeaponTipSocket);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(WeaponSweep), false, this);
	Params.AddIgnoredActor(this);

	for (int32 i = 0; i <= SampleCount; i++)
	{
		const float Alpha = static_cast<float>(i) / static_cast<float>(SampleCount);
		const FVector LastPos     = FMath::Lerp(LastWeaponBasePos, LastWeaponTipPos, Alpha);
		const FVector CurrentPos  = FMath::Lerp(CurrentBase, CurrentTip, Alpha);
		if (LastPos.Equals(CurrentPos, KINDA_SMALL_NUMBER))
		{
			continue;
		}
		FHitResult Hit;
		const bool bHit = GetWorld()->SweepSingleByChannel(
			Hit,
			LastPos,
			CurrentPos,
			FQuat::Identity,
			ECC_Pawn,
			FCollisionShape::MakeSphere(WeaponSweepRadius),
			Params);
		DrawDebugLine(GetWorld(), LastPos, CurrentPos, FColor::Red, false, 0.5f, 0, 2.f);
		DrawDebugSphere(GetWorld(), CurrentPos, WeaponSweepRadius, 8, FColor::Yellow, false, 0.5f);
		if (bHit && Hit.GetActor())
		{
			if (!Cast<APawn>(Hit.GetActor()))
			{
				continue;
			}
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1, 2.f, FColor::Green,
					FString::Printf(TEXT("Hit: %s"), *Hit.GetActor()->GetName()));
			}
			ApplyMeleeDamageToActor(Hit.GetActor());
		}
	}
	LastWeaponBasePos = CurrentBase;
	LastWeaponTipPos  = CurrentTip;
}

void AWGasCharacterBase::EndWeaponSweep()
{
	bWeaponSweepActive = false;
	MeleeHitActorsThisSwing.Empty();
}

USkeletalMeshComponent* AWGasCharacterBase::GetWeaponTraceMesh() const
{
	return Weapon;
}

void AWGasCharacterBase::ApplyMeleeDamageToActor(AActor* HitActor)
{
	if (!HitActor||MeleeHitActorsThisSwing.Contains(HitActor))return;
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;
	const FGameplayTag MeleeLightTag = FWGasGameplayTags::Get().Abilities_Attack_Melee;
	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (!Spec.IsActive() || !Spec.Ability) continue;
		// GA CDO 上的 Ability Tags
		if (!Spec.Ability->GetAssetTags().HasTagExact(MeleeLightTag)) continue;
		UWGasDamageGameplayAbility* DamageGA =
			Cast<UWGasDamageGameplayAbility>(Spec.GetPrimaryInstance());
		if (!DamageGA) continue;
		DamageGA->CauseDamage(HitActor, DamageGA->GetCurrentDamage());
		MeleeHitActorsThisSwing.Add(HitActor);
		return;
	}
}
