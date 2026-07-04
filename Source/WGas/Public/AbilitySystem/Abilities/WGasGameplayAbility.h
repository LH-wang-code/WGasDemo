// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "WGasGameplayAbility.generated.h"

class AWGasCharacterBase;
class UAbilitySystemComponent;

UCLASS()
class WGAS_API UWGasGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FGameplayTag StartupInputTag;

protected:
	AWGasCharacterBase* GetWGasCharacterFromActorInfo() const;
	UAbilitySystemComponent* GetWGasASCFromActorInfo() const;
};
