// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/WGasEnemyWidgetController.h"

#include "AbilitySystem/WGasAttributeSet.h"

void UWGasEnemyWidgetController::BroadcastInitialValues()
{
	if (!GetWGasAS())
	{
		return;
	}
	OnHealthChanged.Broadcast(GetWGasAS()->GetHealth());
	OnMaxHealthChanged.Broadcast(GetWGasAS()->GetMaxHealth());
	OnPoiseChanged.Broadcast(GetWGasAS()->GetPoise());
	OnMaxPoiseChanged.Broadcast(GetWGasAS()->GetMaxPoise());
}

void UWGasEnemyWidgetController::BindCallbacksToDependencies()
{
	if (!AbilitySystemComponent || !GetWGasAS())
	{
		return;
	}
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetWGasAS()->GetHealthAttribute()).AddLambda(
	[this](const FOnAttributeChangeData& Data)
	{
		OnHealthChanged.Broadcast(Data.NewValue);
	});

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetWGasAS()->GetMaxHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnMaxHealthChanged.Broadcast(Data.NewValue);
		});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetWGasAS()->GetPoiseAttribute()).AddLambda(
[this](const FOnAttributeChangeData& Data)
		{
			OnPoiseChanged.Broadcast(Data.NewValue);
		});

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetWGasAS()->GetMaxPoiseAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnMaxPoiseChanged.Broadcast(Data.NewValue);
		});
}
