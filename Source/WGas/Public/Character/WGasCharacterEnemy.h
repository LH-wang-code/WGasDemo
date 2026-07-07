// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Character/WGasCharacterBase.h"
#include "WGasCharacterEnemy.generated.h"

/**
 * 
 */
class AWGasAIController;
class UBehaviorTree;
UCLASS()
class WGAS_API AWGasCharacterEnemy : public AWGasCharacterBase
{
	GENERATED_BODY()


public:

	virtual void PossessedBy(AController* NewController)override;
protected:
	ECharacterClass CharacterClass=ECharacterClass::Warrior;
	int32 Level=1;

	//AIController
	UPROPERTY(EditAnywhere,Category="AI")
	TObjectPtr<UBehaviorTree>BehaviorTree;

	UPROPERTY(EditAnywhere, Category = "AI")
	TObjectPtr<AWGasAIController>WGasAIController;
	virtual void BeginPlay() override;

	virtual void InitAbilityActorInfo() override;
	virtual void InitializeDefaultAttributes()const override;

};
