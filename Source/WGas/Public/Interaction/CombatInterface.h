// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatInterface.generated.h"
class UAbilitySystemComponent;
// This class does not need to be modified.
UINTERFACE(MinimalAPI,BlueprintType)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class WGAS_API ICombatInterface
{
	GENERATED_BODY()

public:
	virtual UAbilitySystemComponent* GetDamageableASC() const = 0;

	virtual bool IsAliveForCombat() const { return true; }
};
