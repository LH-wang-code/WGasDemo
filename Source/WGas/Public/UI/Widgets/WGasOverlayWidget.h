// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/WGasUserWidget.h"
#include "WGasOverlayWidget.generated.h"

/**
 * 
 */
class UWGasBossBarWidget;
UCLASS()
class WGAS_API UWGasOverlayWidget : public UWGasUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWGasBossBarWidget> BossBarWidget;
};
