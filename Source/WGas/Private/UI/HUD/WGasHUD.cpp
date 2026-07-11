// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/WGasHUD.h"
#include "UI/WidgetController/OverlayWGasWidgetController.h"
#include "UI/WidgetController/WGasEnemyWidgetController.h"
#include "UI/Widgets/WGasUserWidget.h"
#include "AbilitySystem/WGasAbilitySystemComponent.h"
#include "AbilitySystem/WGasAttributeSet.h"
#include "Character/WGasCharacterBase.h"
#include "UI/Widgets/WGasOverlayWidget.h"
#include "UI/Widgets/WGasBossBarWidget.h"
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

UWGasEnemyWidgetController* AWGasHUD::CreateEnemyWidgetController(APlayerController* PC,
	UAbilitySystemComponent* EnemyASC, UAttributeSet* EnemyAS)
{
	if (!PC || !EnemyASC || !EnemyAS)
	{
		return nullptr;
	}
	if (!EnemyWidgetControllerClass)
	{
		return nullptr;
	}

	UWGasEnemyWidgetController* EnemyWidgetController =
		NewObject<UWGasEnemyWidgetController>(this, EnemyWidgetControllerClass);
	const FWidgetControllerParams WidgetParams(PC, EnemyASC, EnemyAS);
	EnemyWidgetController->SetWidgetControllerParams(WidgetParams);
	EnemyWidgetController->BindCallbacksToDependencies();
	return EnemyWidgetController;
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
		return;
	}
	OverlayWidget = Cast<UWGasOverlayWidget>(Widget);
	if (!OverlayWidget)
	{
		return;
	}
	FWidgetControllerParams WidgetParams(PC,ASC,AS);
	UOverlayWGasWidgetController* WidgetController = GetOverlayWidgetController(WidgetParams);
	if (!WidgetController)
	{
		return;
	}
	OverlayWidget->SetWidgetController(WidgetController);
	WidgetController->BroadcastInitialValues();
	Widget->AddToViewport();
}

void AWGasHUD::ShowEnemyTarget(APlayerController* PC, AActor* Enemy)
{
	if (!PC || !Enemy)
	{
		return;
	}
	AWGasCharacterBase* EnemyCharacter = Cast<AWGasCharacterBase>(Enemy);
	if (!EnemyCharacter || !EnemyCharacter->IsAliveForCombat())
	{
		HideEnemyTarget();
		return;
	}
	if (!OverlayWidget || !OverlayWidget->BossBarWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShowEnemyTarget: OverlayWidget or BossBarWidget is null"));
		return;
	}

	// Timer 每帧重复 Show 时，同一敌人不要重复 Create Controller，否则 ASC 上会堆 Lambda、UI Bind 也会被冲掉
	if (TrackedEnemy == Enemy && OverlayWidget->BossBarWidget->WidgetController != nullptr)
	{
		OverlayWidget->BossBarWidget->SetVisibility(ESlateVisibility::Visible);
		return;
	}

	HideEnemyTarget();
	TrackedEnemy = Enemy;

	UAbilitySystemComponent* EnemyASC = EnemyCharacter->GetAbilitySystemComponent();
	UAttributeSet* EnemyAS = EnemyCharacter->GetAttributeSet();
	UWGasEnemyWidgetController* Controller = CreateEnemyWidgetController(PC, EnemyASC, EnemyAS);
	if (!Controller)
	{
		TrackedEnemy = nullptr;
		return;
	}

	OverlayWidget->BossBarWidget->SetWidgetController(Controller);
	Controller->BroadcastInitialValues();
	OverlayWidget->BossBarWidget->SetVisibility(ESlateVisibility::Visible);
}

void AWGasHUD::HideEnemyTarget()
{
	if (OverlayWidget && OverlayWidget->BossBarWidget)
	{
		OverlayWidget->BossBarWidget->SetWidgetController(nullptr);
		OverlayWidget->BossBarWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	TrackedEnemy = nullptr;
}
