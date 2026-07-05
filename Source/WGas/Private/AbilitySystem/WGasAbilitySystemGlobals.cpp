// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/WGasAbilitySystemGlobals.h"

#include "WGasEffectTypes.h"

FGameplayEffectContext* UWGasAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FWGasGameplayEffectContext();
}
