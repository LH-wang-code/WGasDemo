// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTService_ReadPlayerAttack.h"

#include "AbilitySystemComponent.h"
#include "WGasGameplayTags.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_ReadPlayerAttack::UBTService_ReadPlayerAttack()
{
	NodeName = TEXT("Read Player Attack");
	bNotifyTick = true;
	Interval = 0.05f;
	RandomDeviation = 0.f;
	TargetActorKey.AddObjectFilter(this,GET_MEMBER_NAME_CHECKED(
		UBTService_ReadPlayerAttack,
		TargetActorKey),
	AActor::StaticClass());
	PlayerAttackStartupKey.AddBoolFilter(this,GET_MEMBER_NAME_CHECKED(
		UBTService_ReadPlayerAttack,
		PlayerAttackStartupKey));
}

void UBTService_ReadPlayerAttack::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (const UBlackboardData* BlackboardAsset =GetBlackboardAsset())
	{
		TargetActorKey.ResolveSelectedKey(*BlackboardAsset);
		PlayerAttackStartupKey.ResolveSelectedKey(*BlackboardAsset);
	}
}

void UBTService_ReadPlayerAttack::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	UBlackboardComponent* Blackboard =OwnerComp.GetBlackboardComponent();
	if (!Blackboard || !TargetActorKey.IsSet()|| !PlayerAttackStartupKey.IsSet())
	{
		UE_LOG(LogTemp, Error,
			TEXT("ReadPlayerAttack Service invalid setup: Blackboard=%d TargetKey=%d StartupKey=%d"),
			Blackboard != nullptr, TargetActorKey.IsSet(), PlayerAttackStartupKey.IsSet());
		return;
	}
	AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
	// TargetActor 是 Blackboard 中的玩家 Actor；直接取组件可避免未实现接口时蓝图库返回空。
	UAbilitySystemComponent* TargetASC = TargetActor
		? TargetActor->FindComponentByClass<UAbilitySystemComponent>()
		: nullptr;
	const FWGasGameplayTags& Tags = FWGasGameplayTags::Get();
	// 不同的玩家攻击 GA 可能没有单独设置 Lighting Tag，但都会进入 Active。
	// 对 Boss 来说，这两个阶段都应视作可以读取并触发格挡的攻击时机。
	const bool bPlayerStartingAttack = TargetASC &&
		((Tags.State_Attacking_Lighting.IsValid()
			&& TargetASC->HasMatchingGameplayTag(Tags.State_Attacking_Lighting))
		|| (Tags.State_Attacking_Active.IsValid()
			&& TargetASC->HasMatchingGameplayTag(Tags.State_Attacking_Active)));
	Blackboard->SetValueAsBool(PlayerAttackStartupKey.SelectedKeyName,bPlayerStartingAttack);
	if (bPlayerStartingAttack)
	{
		UE_LOG(LogTemp, Warning, TEXT("ReadPlayerAttack Blackboard=true Target=%s"),
			TargetActor ? *TargetActor->GetName() : TEXT("None"));
	}
}
