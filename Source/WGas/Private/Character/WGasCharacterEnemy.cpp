// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/WGasCharacterEnemy.h"

#include "AbilitySystem/WGasAbilitySystemFunctionLibrary.h"
#include "AbilitySystem/WGasAttributeSet.h"

void AWGasCharacterEnemy::BeginPlay()
{
	Super::BeginPlay();
	InitAbilityActorInfo();
	
}

void AWGasCharacterEnemy::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	InitializeDefaultAttributes();
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

void AWGasCharacterEnemy::InitializeDefaultAttributes() const
{
	UWGasAbilitySystemFunctionLibrary::InitializeDefaultCharacterClassInfo(this,CharacterClass,Level,AbilitySystemComponent);


}
