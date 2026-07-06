// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/WGasWidgetController.h"
#include "StaminaBarWGasWidgetController.generated.h"

/**
 * 
 */


UCLASS(BlueprintType,Blueprintable)
class WGAS_API UStaminaBarWGasWidgetController : public UWGasWidgetController
{
	GENERATED_BODY()



public:
	virtual void BroadcastInitialValues()override;
	virtual void BindCallbacksToDependencies()override;


	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnAttributeChangedSignature OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnAttributeChangedSignature OnMaxStaminaChanged;

};
