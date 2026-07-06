// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WGasGameplayAbility.h"
#include "WGasPassiveGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class WGAS_API UWGasPassiveGameplayAbility : public UWGasGameplayAbility
{
	GENERATED_BODY()

public:
	UWGasPassiveGameplayAbility();
protected:

	UPROPERTY(EditDefaultsOnly, Category = "Passive")
	TSubclassOf<UGameplayEffect> PassiveGameplayEffectClass;
	void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilitySpec& Spec)override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilitySpec& Spec) override;

private:
	FActiveGameplayEffectHandle PassiveEffectHandle;
};
