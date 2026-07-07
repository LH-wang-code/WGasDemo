// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_PickPatrolLocation.generated.h"

/**
 * 
 */
UCLASS()
class WGAS_API UBTT_PickPatrolLocation : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTT_PickPatrolLocation();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Blackboard")
	FName HomeLocationKeyName = TEXT("HomeLocation");
	UPROPERTY(EditDefaultsOnly, Category = "Blackboard")
	FName PatrolLocationKeyName = TEXT("PatrolLocation");
	UPROPERTY(EditDefaultsOnly, Category = "Patrol", meta = (ClampMin = "100"))
	float PatrolRadius = 1000.f;
};
