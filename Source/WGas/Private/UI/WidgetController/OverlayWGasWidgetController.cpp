// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/OverlayWGasWidgetController.h"

#include "AbilitySystem/WGasAttributeSet.h"

void UOverlayWGasWidgetController::BroadcastInitialValues()
{
	//UI这部分直接通过ASC获取属性值饼进行广播
	OnHealthChanged.Broadcast(GetWGasAS()->GetHealth());
	OnMaxHealthChanged.Broadcast(GetWGasAS()->GetMaxHealth());
	OnManaChanged.Broadcast(GetWGasAS()->GetMana());
	OnMaxManaChanged.Broadcast(GetWGasAS()->GetMaxMana());
	OnMomentumChanged.Broadcast(GetWGasAS()->GetMomentum());
	OnMaxMomentumChanged.Broadcast(GetWGasAS()->GetMaxMomentum());
}

void UOverlayWGasWidgetController::BindCallbacksToDependencies()
{
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		GetWGasAS()->GetHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnHealthChanged.Broadcast(Data.NewValue);
		});

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetWGasAS()->GetMaxHealthAttribute()).AddLambda(
	[this](const FOnAttributeChangeData& Data) 
	{
		OnMaxHealthChanged.Broadcast(Data.NewValue);
	});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetWGasAS()->GetManaAttribute()).AddLambda(
	[this](const FOnAttributeChangeData& Data)
	{
		OnManaChanged.Broadcast(Data.NewValue);
	});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetWGasAS()->GetMaxManaAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnMaxManaChanged.Broadcast(Data.NewValue);
		});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetWGasAS()->GetMomentumAttribute()).AddLambda(
	[this](const FOnAttributeChangeData& Data)
	{
		OnMomentumChanged.Broadcast(Data.NewValue);
	});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetWGasAS()->GetMaxMomentumAttribute()).AddLambda(
	[this](const FOnAttributeChangeData& Data)
	{
		OnMaxMomentumChanged.Broadcast(Data.NewValue);
	});
}

void UOverlayWGasWidgetController::HealthChanged(const FOnAttributeChangeData& Data) const
{
}

void UOverlayWGasWidgetController::MaxHealthChanged(const FOnAttributeChangeData& Data) const
{
}

void UOverlayWGasWidgetController::ManaChanged(const FOnAttributeChangeData& Data) const
{
}

void UOverlayWGasWidgetController::MaxManaChanged(const FOnAttributeChangeData& Data) const
{
}
