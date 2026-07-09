// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTT_SelectBossAttack.h"

#include "AIController.h"
#include "AbilitySystem/Data/BossAttackInfo.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/WGasCharacterEnemy.h"

EBTNodeResult::Type UBTT_SelectBossAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Pawn || !Blackboard)
	{
		return EBTNodeResult::Failed;
	}

	AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TEXT("TargetActor")));
	AWGasCharacterEnemy* Enemy = Cast<AWGasCharacterEnemy>(Pawn);
	if (!TargetActor || !Enemy || !Enemy->AttackSet)
	{
		return EBTNodeResult::Failed;
	}

	const TArray<FBossAttackInfomation>& Attacks = Enemy->AttackSet->Attacks;
	if (Attacks.IsEmpty())
	{
		return EBTNodeResult::Failed;
	}

	FVector ToTarget = TargetActor->GetActorLocation() - Pawn->GetActorLocation();
	ToTarget.Z = 0.f;
	const float Distance = ToTarget.Size();

	TArray<int32> ValidAttackIndices;
	ValidAttackIndices.Reserve(Attacks.Num());
	for (int32 Index = 0; Index < Attacks.Num(); ++Index)
	{
		const FBossAttackInfomation& Info = Attacks[Index];
		if (!Info.AbilityTag.IsValid())
		{
			continue;
		}
		if (Distance >= Info.MinRange && Distance <= Info.MaxRange)
		{
			ValidAttackIndices.Add(Index);
		}
	}

	if (ValidAttackIndices.IsEmpty())
	{
		return EBTNodeResult::Failed;
	}

	int32 SelectedIndex = ValidAttackIndices[0];
	if (ValidAttackIndices.Num() > 1)
	{
		float TotalWeight = 0.f;
		for (const int32 Index : ValidAttackIndices)
		{
			TotalWeight += FMath::Max(Attacks[Index].Weight, 0.f);
		}

		if (TotalWeight > KINDA_SMALL_NUMBER)
		{
			const float Roll = FMath::FRandRange(0.f, TotalWeight);
			float Accumulator = 0.f;
			for (const int32 Index : ValidAttackIndices)
			{
				Accumulator += FMath::Max(Attacks[Index].Weight, 0.f);
				if (Roll <= Accumulator)
				{
					SelectedIndex = Index;
					break;
				}
			}
		}
	}

	const FBossAttackInfomation& Selected = Attacks[SelectedIndex];
	Blackboard->SetValueAsName(TEXT("SelectedAttackTag"), Selected.AbilityTag.GetTagName());
	UE_LOG(LogTemp, Warning, TEXT("Select: Dist=%.0f Tag=%s"), Distance, *Selected.AbilityTag.ToString());

	return EBTNodeResult::Succeeded;
}
