// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WGasGameplayAbility.h"
#include "WGasBossParry.generated.h"

/**
 * 
 */
UCLASS()
class WGAS_API UWGasBossParry : public UWGasGameplayAbility
{
	GENERATED_BODY()
public:
	UWGasBossParry();
	virtual void ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled) override;

	UFUNCTION(BlueprintCallable, Category = "Boss|Parry")
	void OpenParryWindow();

	UFUNCTION(BlueprintCallable, Category = "Boss|Parry")
	void CloseParryWindow();

	UFUNCTION(BlueprintPure, Category = "Boss|Parry")
	bool HasParrySucceeded() const;
	
	UFUNCTION(BlueprintCallable, Category = "Boss|Parry")
	void EndBossParry(bool bWasCancelled = false);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Parry")
	void OnBossParryActivated();
private:
	bool bParryEndHandled = false;
};
