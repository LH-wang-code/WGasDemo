// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "WGasAIController.generated.h"

/**
 * 
 */
UCLASS()
class WGAS_API AWGasAIController : public AAIController
{
	GENERATED_BODY()


protected:
	UPROPERTY()
	TObjectPtr<UBehaviorTree> BehaviorTreeComponent;
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "AI")
	TObjectPtr<UBlackboardComponent>BlckBlackboardComponent;	
};
