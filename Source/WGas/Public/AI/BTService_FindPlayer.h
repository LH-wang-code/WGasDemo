// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlueprintBase.h"
#include "BTService_FindPlayer.generated.h"

/**
 * 
 */

USTRUCT()
struct FBTFindPlayerServiceMemory
{
	GENERATED_BODY()
	bool bHomeLocationSet = false;
};
UCLASS()
class WGAS_API UBTService_FindPlayer : public UBTService_BlueprintBase
{
	GENERATED_BODY()
public:
	UBTService_FindPlayer();
	virtual uint16 GetInstanceMemorySize() const override
	{
		return sizeof(FBTFindPlayerServiceMemory);
	}
protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)override;

	UPROPERTY(EditDefaultsOnly)
	float AggroRadius = 1200.f;
	UPROPERTY(EditDefaultsOnly)
	float AttackRadius = 200.f;
};
