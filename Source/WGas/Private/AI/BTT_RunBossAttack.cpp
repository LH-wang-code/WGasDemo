// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTT_RunBossAttack.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "WGasGameplayTags.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTT_RunBossAttack::UBTT_RunBossAttack()
{
	NodeName = TEXT("Run Boss Attack");
	bNotifyTick = true;
	const FWGasGameplayTags& Tags = FWGasGameplayTags::Get();
	AttackAbilityTag = Tags.Ability_Boss_Melee;
	AttackingStateTag = Tags.State_Boss_Attacking;
}

EBTNodeResult::Type UBTT_RunBossAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	APawn* Pawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	if (!Pawn)
	{
		return EBTNodeResult::Failed;
	}

	UAbilitySystemComponent* ASC = Pawn->FindComponentByClass<UAbilitySystemComponent>();

	if (!ASC || !AttackAbilityTag.IsValid())
	{
		return EBTNodeResult::Failed;
	}
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	const FName TagName = BB ? BB->GetValueAsName(TEXT("SelectedAttackTag")) : NAME_None;
	FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag(TagName, false);
	if (!AttackTag.IsValid())return EBTNodeResult::Failed;
	
	FGameplayTagContainer AbilityTags;
	AbilityTags.AddTag(AttackTag);
	if (!ASC->TryActivateAbilitiesByTag(AbilityTags))
	{
		return EBTNodeResult::Failed;
	}
	UE_LOG(LogTemp, Warning, TEXT("Run TryActivate %s "), *AttackTag.ToString());
	return EBTNodeResult::InProgress;
}

void UBTT_RunBossAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	APawn* Pawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	if (!Pawn)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	UAbilitySystemComponent* ASC = Pawn->FindComponentByClass<UAbilitySystemComponent>();
	if (!ASC)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (AttackingStateTag.IsValid() && ASC->HasMatchingGameplayTag(AttackingStateTag))
	{
		return;
	}

	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
}

EBTNodeResult::Type UBTT_RunBossAttack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	APawn* Pawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	if (Pawn)
	{
		if (UAbilitySystemComponent* ASC = Pawn->FindComponentByClass<UAbilitySystemComponent>())
		{
			if (AttackAbilityTag.IsValid())
			{
				FGameplayTagContainer GasTags;
				GasTags.AddTag(AttackAbilityTag);
				ASC->CancelAbilities(&GasTags);
			}
			if (AttackingStateTag.IsValid())
			{
				ASC->RemoveLooseGameplayTag(AttackingStateTag);
			}
		}
	}

	return Super::AbortTask(OwnerComp, NodeMemory);
}
