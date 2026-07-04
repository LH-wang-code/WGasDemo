// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	virtual void BeginPlay() override;

	virtual void InitAbilityActorInfo() override;
	
};
