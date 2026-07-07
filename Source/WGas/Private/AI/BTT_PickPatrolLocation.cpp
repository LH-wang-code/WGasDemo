// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTT_PickPatrolLocation.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTT_PickPatrolLocation::UBTT_PickPatrolLocation()
{
}

EBTNodeResult::Type UBTT_PickPatrolLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;
	if (!Blackboard || !Pawn)
	{
		return EBTNodeResult::Failed;
	}
	FVector Origin = Blackboard->GetValueAsVector(HomeLocationKeyName);
	if (Origin.IsNearlyZero())
	{
		Origin = Pawn->GetActorLocation();
	}
	FVector RandomOffset = FMath::VRand() * FMath::FRandRange(100.f, PatrolRadius);
	RandomOffset.Z = 0.f;
	const FVector RandomPoint = Origin + RandomOffset;

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(Pawn->GetWorld());
	if (!NavSys)
	{
		return EBTNodeResult::Failed;
	}
	FNavLocation NavLocation;
	const bool bFound = NavSys->ProjectPointToNavigation(
		RandomPoint,
		NavLocation,
		FVector(PatrolRadius, PatrolRadius, 300.f));
	UE_LOG(LogTemp, Warning, TEXT("Origin=%s RandomPoint=%s Pawn=%s"),
	*Origin.ToString(), *RandomPoint.ToString(), *Pawn->GetActorLocation().ToString());
	if (!bFound)
	{
		return EBTNodeResult::Failed;
	}
	Blackboard->SetValueAsVector(PatrolLocationKeyName, NavLocation.Location);
	return EBTNodeResult::Succeeded;
}
