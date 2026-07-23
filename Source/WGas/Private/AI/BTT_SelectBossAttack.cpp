// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTT_SelectBossAttack.h"

#include "AIController.h"
#include "AbilitySystem/Data/BossAttackInfo.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Interaction/BossCombatInterface.h"
#include "WGasGameplayTags.h"

EBTNodeResult::Type UBTT_SelectBossAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Pawn || !Blackboard)
	{
		return EBTNodeResult::Failed;
	}

	if (UAbilitySystemComponent* ASC = Pawn->FindComponentByClass<UAbilitySystemComponent>())
	{
		const FGameplayTag& ParriedTag = FWGasGameplayTags::Get().State_Parried;
		if (ParriedTag.IsValid() && ASC->HasMatchingGameplayTag(ParriedTag))
		{
			return EBTNodeResult::Failed;
		}
	}

	AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TEXT("TargetActor")));
	IBossCombatInterface* BossCombat = Cast<IBossCombatInterface>(Pawn);
	const UBossAttackInfo* AttackSet = BossCombat ? BossCombat->GetBossAttackSet() : nullptr;
	if (!TargetActor || !AttackSet)
	{
		return EBTNodeResult::Failed;
	}

	const TArray<FBossAttackInfomation>& Attacks = AttackSet->Attacks;
	if (Attacks.IsEmpty())
	{
		return EBTNodeResult::Failed;
	}
	if (BossCombat)
	{
		const FWGasGameplayTags& Tags = FWGasGameplayTags::Get();

		if (BossCombat->IsBossGuardReleaseReady()
			&& Tags.Ability_Boss_Greatsword_GuardRelease.IsValid())
		{
			Blackboard->SetValueAsName(
				TEXT("SelectedAttackTag"),
				Tags.Ability_Boss_Greatsword_GuardRelease.GetTagName());

			return EBTNodeResult::Succeeded;
		}
	}
	FVector ToTarget = TargetActor->GetActorLocation() - Pawn->GetActorLocation();
	ToTarget.Z = 0.f;
	const float Distance = ToTarget.Size();
	const FWGasGameplayTags& WGasTags = FWGasGameplayTags::Get();
	// 由 BTService_ReadPlayerParry 写入黑板：选招任务只消费黑板数据。
	const bool bPlayerAttackStartup = Blackboard->GetValueAsBool(TEXT("bPlayerAttackStartup"));
	const bool bCanReadParry =bPlayerAttackStartup&& Distance >= 90.f&& Distance <= 260.f&& FMath::FRand() <= 1.f ;
	if (bCanReadParry)
	{
		const bool bIsPhase2 = BossCombat && BossCombat->IsBossPhase2();

		const FGameplayTag GuardAbilityTag = bIsPhase2
			? WGasTags.Ability_Boss_Greatsword_Guard
			: WGasTags.Ability_Boss_Parry;

		if (GuardAbilityTag.IsValid())
		{
			Blackboard->SetValueAsName(
				TEXT("SelectedAttackTag"),
				GuardAbilityTag.GetTagName());

			UE_LOG(LogTemp, Warning, TEXT("Select Guard: Phase=%d Tag=%s"),
				bIsPhase2, *GuardAbilityTag.ToString());
			return EBTNodeResult::Succeeded;
		}
	}


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
