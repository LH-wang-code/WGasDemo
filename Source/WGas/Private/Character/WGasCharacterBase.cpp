// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/WGasCharacterBase.h"

#include "WGasGameplayTags.h"
#include "AbilitySystem/WGasAbilitySystemComponent.h"
#include "AbilitySystem/WGasAttributeSet.h"
#include "AbilitySystem/Abilities/WGasDamageGameplayAbility.h"
#include "AbilitySystem/Abilities/WGasBlock.h"
#include "AbilitySystem/Abilities/WGasDodge.h"
#include "Abilities/GameplayAbility.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AWGasCharacterBase::AWGasCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UWGasAbilitySystemComponent>("AbilitySystemComponent");
	AttributeSet = CreateDefaultSubobject<UWGasAttributeSet>("AttributeSet");
	CombatComponent = CreateDefaultSubobject<UWGasCombatComponent>(TEXT("CombatComponent"));
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

UAbilitySystemComponent* AWGasCharacterBase::GetDamageableASC() const
{
	return AbilitySystemComponent;
}

bool AWGasCharacterBase::IsAliveForCombat() const
{
	const UWGasAttributeSet* AS = Cast<UWGasAttributeSet>(AttributeSet);
	if (!AS)
	{
		return true;  
	}
	return AS->GetHealth() > 0.f;
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
	if (CombatComponent)CombatComponent->BeginWeaponSweep();
}

void AWGasCharacterBase::EndWeaponSweep()
{
	if (CombatComponent)CombatComponent->EndWeaponSweep();
}

USkeletalMeshComponent* AWGasCharacterBase::GetWeaponTraceMesh() const
{
	if (Weapon && Weapon->GetSkinnedAsset())
	{
		return Weapon;
	}
	return GetMesh();
}

void AWGasCharacterBase::NotifyDeath()
{
	if (bDeathHandled) return;
	bDeathHandled = true;
	ApplyDeathState();    // 通用
	HandleDeathExtras();  // Hero/Enemy 各自 override
	OnDeath();
}

void AWGasCharacterBase::OnDeath_Implementation()
{
}

void AWGasCharacterBase::ApplyDeathState()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	const FWGasGameplayTags& WGasTags = FWGasGameplayTags::Get();
	if (ASC)
	{
		ASC->CancelAllAbilities();
		if (WGasTags.State_Dead.IsValid())
		{
			ASC->AddLooseGameplayTag(WGasTags.State_Dead);
		}
	}
	if (CombatComponent)
	{
		CombatComponent->EndWeaponSweep();
	}
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->StopMovementImmediately();
		Move->DisableMovement();
	}
	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AWGasCharacterBase::HandleDeathExtras()
{
}

void AWGasCharacterBase::EndDodgeFromAnimation()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (!Spec.IsActive() || !Spec.Ability)
		{
			continue;
		}

		if (!Spec.Ability->GetClass()->IsChildOf(UWGasDodge::StaticClass()))
		{
			continue;
		}

		if (UGameplayAbility* Instance = Spec.GetPrimaryInstance())
		{
			if (UWGasDodge* Dodge = Cast<UWGasDodge>(Instance))
			{
				Dodge->EndDodgeActivePhase();
				return;
			}
		}
	}
}

void AWGasCharacterBase::EndBlockFromAnimation()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	if (UWGasBlock* Block = UWGasBlock::GetActiveBlock(ASC))
	{
		Block->EndBlockActivePhase();
	}
}
