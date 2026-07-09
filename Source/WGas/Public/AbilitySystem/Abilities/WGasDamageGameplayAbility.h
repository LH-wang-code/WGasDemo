// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WGasGameplayAbility.h"
#include "WGasEffectTypes.h"
#include "WGasDamageGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class WGAS_API UWGasDamageGameplayAbility : public UWGasGameplayAbility
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	TSubclassOf<UGameplayEffect>DamageEffectClass;


	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	FGameplayTag DamageType;
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float Damage;

	FDamagePayLoad BuildDamagePayload() const;
	void RegisterDamageWindow();
	void UnregisterDamageWindow();
public:
	FDamageEffectParams MakeDamageEffectParams(AActor* TargetActor, float OverrideDamage = -1.f) const;

	UFUNCTION(BlueprintCallable, Category = "Damage")
	void CauseDamage(AActor* TargetActor, float OverrideDamage = -1.f);
	float GetCurrentDamage(){return Damage;}
};
