// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/WGasDamageGameplayAbility.h"
#include "GameplayAbilities/Public/AbilitySystemBlueprintLibrary.h"

#include "WGasEffectTypes.h"
#include "AbilitySystem/WGasAbilitySystemFunctionLibrary.h"
#include "Character/WGasCharacterBase.h"

void UWGasDamageGameplayAbility::CauseDamage(AActor* TargetActor,float OverrideDamage)
{
	UWGasAbilitySystemFunctionLibrary::ApplyDamageEffectParams(
		  MakeDamageEffectParams(TargetActor, OverrideDamage));
}

FDamage UWGasDamageGameplayAbility::BuildDamagePayload() const
{
	FDamagePayLoad Payload;
	Payload.DamageEffectClass = DamageEffectClass;
	Payload.DamageType        = DamageType;
	Payload.BaseDamage        = Damage > 0.f ? Damage : 5.f;
	Payload.SourceASC         = GetAbilitySystemComponentFromActorInfo();
	Payload.Instigator        = GetAvatarActorFromActorInfo();
	return Payload;
}
void UWGasDamageGameplayAbility::RegisterDamageWindow()
{
	AWGasCharacterBase* Character = GetWGasCharacterFromActorInfo();
	if (!Character || !Character->CombatComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("RegisterDamageWindow: 缺少 Character 或 CombatComponent"));
		return;
	}

	const FDamagePayLoad Payload = BuildDamagePayload();
	if (!Payload.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("RegisterDamageWindow: Payload 无效，检查 GA 的 DamageEffectClass / DamageType / Damage"));
		return;
	}

	Character->CombatComponent->BeginDamageWindow(Payload);
}
void UWGasDamageGameplayAbility::UnregisterDamageWindow()
{
	AWGasCharacterBase* Character = GetWGasCharacterFromActorInfo();
	if (!Character || !Character->CombatComponent)
	{
		return;
	}

	Character->CombatComponent->EndDamageWindow();
}
FDamageEffectParams UWGasDamageGameplayAbility::MakeDamageEffectParams(AActor* TargetActor, float OverrideDamage) const
{
	FDamageEffectParams Params;
	Params.WorldContextObject=GetAvatarActorFromActorInfo();
	Params.DamageGameplayEffectClass=DamageEffectClass;
	Params.SourceAbilitySystemComponent=GetAbilitySystemComponentFromActorInfo();
	Params.TargetAbilitySystemComponent =UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (OverrideDamage >= 0.f)
	{
		Params.BaseDamage = OverrideDamage;
	}
	else
	{
		Params.BaseDamage = Damage > 0.f ? Damage : 5.f;
	}
	Params.DamageType=DamageType;
	return Params;
}
