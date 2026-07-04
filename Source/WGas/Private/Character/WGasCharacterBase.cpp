// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/WGasCharacterBase.h"

#include "AbilitySystem/WGasAbilitySystemComponent.h"
#include "AbilitySystem/WGasAttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"

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
