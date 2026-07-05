// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/WGasWidgetController.h"
#include "OverlayWGasWidgetController.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeChangedSignature,float,NewValue);


UCLASS(BlueprintType,Blueprintable)
class WGAS_API UOverlayWGasWidgetController : public UWGasWidgetController
{
	GENERATED_BODY()
public:
	virtual void BroadcastInitialValues()override;
	virtual void BindCallbacksToDependencies()override;

	UPROPERTY(BlueprintAssignable,Category="GAS|Attributes")
	FOnAttributeChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnAttributeChangedSignature OnMaxHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnAttributeChangedSignature OnManaChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnAttributeChangedSignature OnMaxManaChanged;


protected:
	void HealthChanged(const FOnAttributeChangeData& Data)const;
	void MaxHealthChanged(const FOnAttributeChangeData& Data)const;
	void ManaChanged(const FOnAttributeChangeData& Data)const;
	void MaxManaChanged(const FOnAttributeChangeData& Data)const;
};
