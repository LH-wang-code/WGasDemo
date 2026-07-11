// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "WGasHUD.generated.h"

/**
 * 
 */
class UWGasOverlayWidget;
class UOverlayWGasWidgetController;
class UWGasEnemyWidgetController;
class UAbilitySystemComponent;
class UAttributeSet;
struct FWidgetControllerParams;
UCLASS()
class WGAS_API AWGasHUD : public AHUD
{
	GENERATED_BODY()
public:
	UPROPERTY()
	TObjectPtr<UWGasOverlayWidget>OverlayWidget;


	UOverlayWGasWidgetController* GetOverlayWidgetController(const FWidgetControllerParams& WCParams);

	UFUNCTION(BlueprintCallable, Category = "UI")
	UWGasEnemyWidgetController* CreateEnemyWidgetController(APlayerController* PC,UAbilitySystemComponent* EnemyASC,UAttributeSet* EnemyAS);
	void InitOverlay(APlayerController* PC,  UAbilitySystemComponent* ASC, UAttributeSet* AS);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowEnemyTarget(APlayerController* PC, AActor* Enemy);
	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideEnemyTarget();

private:
	//当前怪物，后续扩展可能用到
	UPROPERTY()
	TObjectPtr<AActor> TrackedEnemy;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UWGasOverlayWidget> OverlayWidgetClass;
	UPROPERTY()
	TObjectPtr<UOverlayWGasWidgetController>OverlayWGasWidgetController;


	UPROPERTY(EditAnywhere)
	TSubclassOf<UOverlayWGasWidgetController>OverlayWidgetControllerClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UWGasEnemyWidgetController> EnemyWidgetControllerClass;

};
