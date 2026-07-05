// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/WGasWidgetController.h"

#include "AbilitySystem/WGasAbilitySystemComponent.h"
#include "AbilitySystem/WGasAttributeSet.h"
void UWGasWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& WCParams)
{
	PlayerController=WCParams.PlayerController;
	AbilitySystemComponent=WCParams.AbilitySystemComponent;
	AttributeSet=WCParams.AttributeSet;
}

void UWGasWidgetController::BroadcastInitialValues()
{
}

void UWGasWidgetController::BindCallbacksToDependencies()
{
}

AWGasPlayerController* UWGasWidgetController::GetWGasPC()
{
	if (WGasPlayerController==nullptr)
	{
		WGasPlayerController=Cast<AWGasPlayerController>(PlayerController);
	}
	return WGasPlayerController;
}

UWGasAbilitySystemComponent* UWGasWidgetController::GetWGasASC()
{
	if (WGasAbilitySystemComponent==nullptr)
	{
		WGasAbilitySystemComponent=Cast<UWGasAbilitySystemComponent>(AbilitySystemComponent);
	}
	return WGasAbilitySystemComponent;
}

UWGasAttributeSet* UWGasWidgetController::GetWGasAS()
{
	if (WGasAttributeSet==nullptr)
	{
		WGasAttributeSet=Cast<UWGasAttributeSet>(AttributeSet);
	}
	return WGasAttributeSet;
}
