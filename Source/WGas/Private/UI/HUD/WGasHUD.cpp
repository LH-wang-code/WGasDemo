// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/WGasHUD.h"

#include "UI/WidgetController/OverlayWGasWidgetController.h"
#include "UI/Widgets/WGasUserWidget.h"
#include "AbilitySystem/WGasAbilitySystemComponent.h"
#include "AbilitySystem/WGasAttributeSet.h"

UOverlayWGasWidgetController* AWGasHUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
	if (OverlayWGasWidgetController==nullptr)
	{
		OverlayWGasWidgetController=NewObject<UOverlayWGasWidgetController>(this,OverlayWidgetControllerClass);
		OverlayWGasWidgetController->SetWidgetControllerParams(WCParams);
		OverlayWGasWidgetController->BindCallbacksToDependencies();
	}
	return OverlayWGasWidgetController;
}

void AWGasHUD::InitOverlay(APlayerController* PC, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{

	if (!PC||!ASC||!AS) return;
	if (!OverlayWidgetClass || !OverlayWidgetControllerClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("OverlayWidgetClass or ControllerClass not set"));
		return;
	}
	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
	if (!Widget)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create OverlayWidget"));
		return;
	}
	OverlayWidget = Cast<UWGasUserWidget>(Widget);
	if (!OverlayWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("Cast to UAuraUserWidget failed"));
		return;
	}
	FWidgetControllerParams WidgetParams(PC,ASC,AS);
	UOverlayWGasWidgetController* WidgetController = GetOverlayWidgetController(WidgetParams);
	if (!WidgetController)
	{
		UE_LOG(LogTemp, Error, TEXT("WidgetController is null"));
		return;
	}
	OverlayWidget->SetWidgetController(WidgetController);
	WidgetController->BroadcastInitialValues();
	Widget->AddToViewport();
}
