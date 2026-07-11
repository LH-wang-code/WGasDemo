// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/WGasUserWidget.h"
#include "WGasBossBarWidget.generated.h"

/**
 * 
 */
UCLASS()
class WGAS_API UWGasBossBarWidget : public UWGasUserWidget
{
	GENERATED_BODY()
public:

	virtual void SetWidgetController(UObject* InWidgetController) override;
protected:

	UFUNCTION(BlueprintImplementableEvent, Category = "BossBar")
	void UpdateHealthBar(float Current, float Max);
	UFUNCTION(BlueprintImplementableEvent, Category = "BossBar")
	void UpdatePoiseBar(float Current, float Max);
private:

	UFUNCTION()
	void HandleHealthChanged(float NewValue);
	UFUNCTION()
	void HandleMaxHealthChanged(float NewValue);
	UFUNCTION()
	void HandlePoiseChanged(float NewValue);
	UFUNCTION()
	void HandleMaxPoiseChanged(float NewValue);
	void UnbindController();
	void RefreshHealthBar();
	void RefreshPoiseBar();
	float CurrentHealth = 0.f;
	float CurrentMaxHealth = 1.f;
	float CurrentPoise = 0.f;
	float CurrentMaxPoise = 1.f;
};
