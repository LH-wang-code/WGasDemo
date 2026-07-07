// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTService_FindPlayer.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

UBTService_FindPlayer::UBTService_FindPlayer()
{
	Interval=0.3f;
}

void UBTService_FindPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	UE_LOG(LogTemp, Warning, TEXT("FindPlayer Tick"));
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* OwningPawn = AIController ? AIController->GetPawn() : nullptr;
	if (!Blackboard || !OwningPawn)
		return;
	FBTFindPlayerServiceMemory* Memory = reinterpret_cast<FBTFindPlayerServiceMemory*>(NodeMemory);
	if (!Memory->bHomeLocationSet)
	{
		Blackboard->SetValueAsVector(TEXT("HomeLocation"), OwningPawn->GetActorLocation());
		Memory->bHomeLocationSet = true;
	}
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(OwningPawn, 0);
	UE_LOG(LogTemp, Warning, TEXT("Player=%s"), *GetNameSafe(PlayerPawn));
	if (!IsValid(PlayerPawn))
	{
		Blackboard->ClearValue(TEXT("TargetActor"));
		Blackboard->SetValueAsBool(TEXT("bInAttackRange"), false);
		return;
	}
	float Distance = FVector::Dist(OwningPawn->GetActorLocation(),PlayerPawn->GetActorLocation());
	if (Distance > AggroRadius) 
	{
		Blackboard->ClearValue(TEXT("TargetActor"));
		Blackboard->SetValueAsBool(TEXT("bInAttackRange"), false);
		return;
	}

	Blackboard->SetValueAsObject(TEXT("TargetActor"), PlayerPawn);
	Blackboard->SetValueAsBool(TEXT("bInAttackRange"), Distance <= AttackRadius);
}
