#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType.h"
#include "BTService_ConfrontFaceTarget.generated.h"

UCLASS()
class WGAS_API UBTService_ConfrontFaceTarget : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_ConfrontFaceTarget();

protected:
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	void ApplyFaceRotation(UBehaviorTreeComponent& OwnerComp, float DeltaSeconds) const;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Rotation", meta = (ClampMin = "0"))
	float YawInterpSpeed = 720.f;
};
