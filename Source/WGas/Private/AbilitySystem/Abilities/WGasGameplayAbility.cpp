// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/WGasGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Character/WGasCharacterBase.h"

AWGasCharacterBase* UWGasGameplayAbility::GetWGasCharacterFromActorInfo() const
{
	return Cast<AWGasCharacterBase>(GetAvatarActorFromActorInfo());
}

UAbilitySystemComponent* UWGasGameplayAbility::GetWGasASCFromActorInfo() const
{
	return GetAbilitySystemComponentFromActorInfo();
}
