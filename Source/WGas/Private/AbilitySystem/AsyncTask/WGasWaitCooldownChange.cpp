// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AsyncTask/WGasWaitCooldownChange.h"

#include "AbilitySystemComponent.h"
#include "ActiveGameplayEffectHandle.h"

UWGasWaitCooldownChange* UWGasWaitCooldownChange::WaitForCooldownChange(UAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayTag& InCooldownTag)
{
	//新建冷却，创建任务
	UWGasWaitCooldownChange* WaitCooldownChange=NewObject<UWGasWaitCooldownChange>();
	WaitCooldownChange->ASC = AbilitySystemComponent;
	WaitCooldownChange->CooldownTag = InCooldownTag;
	if (!IsValid(AbilitySystemComponent) || !InCooldownTag.IsValid())
	{
		WaitCooldownChange->EndTask();
		return nullptr;
	}
	//监听标签变换
	AbilitySystemComponent->RegisterGameplayTagEvent(InCooldownTag, EGameplayTagEventType::NewOrRemoved).AddUObject(WaitCooldownChange, &UWGasWaitCooldownChange::CooldownTagChanged);
	//冷却标签添加
	AbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(WaitCooldownChange, &UWGasWaitCooldownChange::OnActiveEffectAdded);
	return WaitCooldownChange;

}

void UWGasWaitCooldownChange::EndTask()
{
	if (!IsValid(ASC))return;
	ASC->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
	SetReadyToDestroy();
	MarkAsGarbage();
}

void UWGasWaitCooldownChange::CooldownTagChanged(FGameplayTag Tag, int32 NewCount)
{
	if (NewCount == 0)
	{
		CooldownEnd.Broadcast(0.f);
	}
}

void UWGasWaitCooldownChange::OnActiveEffectAdded(UAbilitySystemComponent* TargetASC, const FGameplayEffectSpec& Spec,
	FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer AssetTags;
	Spec.GetAllAssetTags(AssetTags);

	FGameplayTagContainer GrantTags;
	Spec.GetAllGrantedTags(GrantTags);

	if (AssetTags.HasTagExact(CooldownTag) || GrantTags.HasTagExact(CooldownTag))
	{
		//获取所有效果的冷却
		FGameplayEffectQuery GameplayEffectQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTag.GetSingleTagContainer());
		
		TArray<float>TimesRemaing= ASC->GetActiveEffectsTimeRemaining(GameplayEffectQuery);
		if (TimesRemaing.Num() > 0)
		{
			float TimeRemaining = TimesRemaing[0];
			for (int32 i = 0; i < TimesRemaing.Num(); i++)
			{
				if (TimesRemaing[i] > TimeRemaining)
				{
					TimeRemaining = TimesRemaing[i];
				}
			}

			CooldownStart.Broadcast(TimeRemaining);
		}
	}
}
