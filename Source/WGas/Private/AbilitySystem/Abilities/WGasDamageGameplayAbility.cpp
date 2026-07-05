// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/WGasDamageGameplayAbility.h"
#include "GameplayAbilities/Public/AbilitySystemBlueprintLibrary.h"

#include "WGasEffectTypes.h"
#include "AbilitySystem/WGasAbilitySystemFunctionLibrary.h"

void UWGasDamageGameplayAbility::CauseDamage(AActor* TargetActor,float OverrideDamage)
{
	UWGasAbilitySystemFunctionLibrary::ApplyDamageEffectParams(
		  MakeDamageEffectParams(TargetActor, OverrideDamage));
}

FDamageEffectParams UWGasDamageGameplayAbility::MakeDamageEffectParams(AActor* TargetActor, float OverrideDamage) const
{
	FDamageEffectParams Params;
	Params.WorldContextObject=GetAvatarActorFromActorInfo();
	Params.DamageGameplayEffectClass=DamageEffectClass;
	Params.SourceAbilitySystemComponent=GetAbilitySystemComponentFromActorInfo();
	Params.TargetAbilitySystemComponent =UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	Params.BaseDamage = (OverrideDamage >= 0.f)
		? OverrideDamage:5.f;
	Params.DamageType=DamageType;
	return Params;
}
