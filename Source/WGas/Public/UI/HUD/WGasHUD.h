// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "WGasHUD.generated.h"

/**
 * 
 */
class UWGasUserWidget;
class UOverlayWGasWidgetController;
class UAbilitySystemComponent;
class UAttributeSet;
struct FWidgetControllerParams;
UCLASS()
class WGAS_API AWGasHUD : public AHUD
{
	GENERATED_BODY()
public:
	UPROPERTY()
	TObjectPtr<UWGasUserWidget>OverlayWidget;


	UOverlayWGasWidgetController* GetOverlayWidgetController(const FWidgetControllerParams& WCParams);


	void InitOverlay(APlayerController* PC,  UAbilitySystemComponent* ASC, UAttributeSet* AS);


private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UWGasUserWidget> OverlayWidgetClass;
	UPROPERTY()
	TObjectPtr<UOverlayWGasWidgetController>OverlayWGasWidgetController;


	UPROPERTY(EditAnywhere)
	TSubclassOf<UOverlayWGasWidgetController>OverlayWidgetControllerClass;
};
