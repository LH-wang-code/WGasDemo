// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/WGasPassiveGameplayAbility.h"

#include "AbilitySystemComponent.h"

UWGasPassiveGameplayAbility::UWGasPassiveGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UWGasPassiveGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
                                                const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
	if (!PassiveGameplayEffectClass)
	{
		return;
	}
	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (!ASC)return;

	if (PassiveEffectHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(PassiveEffectHandle);
		PassiveEffectHandle.Invalidate();
	}
	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);


	const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(PassiveGameplayEffectClass,GetAbilityLevel(), EffectContext);

	if (!SpecHandle.IsValid())
	{
		return;
	}
	PassiveEffectHandle = ASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(),ASC);
}

void UWGasPassiveGameplayAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	if (PassiveEffectHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr)
		{
			ASC->RemoveActiveGameplayEffect(PassiveEffectHandle);
		}
		PassiveEffectHandle.Invalidate();
	}
	Super::OnRemoveAbility(ActorInfo, Spec);
}
