// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_RunBossAttack.generated.h"

/**
 * 
 */
UCLASS()
class WGAS_API UBTT_RunBossAttack : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTT_RunBossAttack();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	FGameplayTag AttackAbilityTag;
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	FGameplayTag AttackingStateTag;
};
