// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Character/WGasCharacterBase.h"
#include "WGasCharacterEnemy.generated.h"

/**
 * 
 */
UCLASS()
class WGAS_API AWGasCharacterEnemy : public AWGasCharacterBase
{
	GENERATED_BODY()


public:
protected:
	ECharacterClass CharacterClass=ECharacterClass::Warrior;
	int32 Level=1;
	virtual void BeginPlay() override;

	virtual void InitAbilityActorInfo() override;
	virtual void InitializeDefaultAttributes()const override;

};
