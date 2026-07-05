// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/WGasCharacterBase.h"

#include "AbilitySystem/WGasAbilitySystemComponent.h"
#include "AbilitySystem/WGasAttributeSet.h"
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
