// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/WGasBossBarWidget.h"

#include "UI/WidgetController/WGasEnemyWidgetController.h"


void UWGasBossBarWidget::SetWidgetController(UObject* InWidgetController)
{
	UnbindController();
	Super::SetWidgetController(InWidgetController);

	UWGasEnemyWidgetController* EnemyWidgetController = Cast<UWGasEnemyWidgetController>(WidgetController);
	if (!EnemyWidgetController)
	{
		return;
	}

	EnemyWidgetController->OnHealthChanged.AddDynamic(this, &UWGasBossBarWidget::HandleHealthChanged);
	EnemyWidgetController->OnMaxHealthChanged.AddDynamic(this, &UWGasBossBarWidget::HandleMaxHealthChanged);
	EnemyWidgetController->OnPoiseChanged.AddDynamic(this, &UWGasBossBarWidget::HandlePoiseChanged);
	EnemyWidgetController->OnMaxPoiseChanged.AddDynamic(this, &UWGasBossBarWidget::HandleMaxPoiseChanged);
}

void UWGasBossBarWidget::UnbindController()
{
	if (UWGasEnemyWidgetController* EnemyWidgetController=Cast<UWGasEnemyWidgetController>(WidgetController))
	{
		EnemyWidgetController->OnHealthChanged.RemoveDynamic(this, &UWGasBossBarWidget::HandleHealthChanged);
		EnemyWidgetController->OnMaxHealthChanged.RemoveDynamic(this, &UWGasBossBarWidget::HandleMaxHealthChanged);
		EnemyWidgetController->OnPoiseChanged.RemoveDynamic(this, &UWGasBossBarWidget::HandlePoiseChanged);
		EnemyWidgetController->OnMaxPoiseChanged.RemoveDynamic(this, &UWGasBossBarWidget::HandleMaxPoiseChanged);
	}
}

void UWGasBossBarWidget::RefreshHealthBar()
{
	UpdateHealthBar(CurrentHealth, CurrentMaxHealth);
}

void UWGasBossBarWidget::RefreshPoiseBar()
{
	UpdatePoiseBar(CurrentPoise, CurrentMaxPoise);
}

void UWGasBossBarWidget::HandleHealthChanged(float NewValue)
{
	CurrentHealth = NewValue;
	RefreshHealthBar();
}

void UWGasBossBarWidget::HandleMaxHealthChanged(float NewValue)
{
	CurrentMaxHealth = FMath::Max(NewValue, 1.f);
	RefreshHealthBar();
}

void UWGasBossBarWidget::HandlePoiseChanged(float NewValue)
{
	CurrentPoise = NewValue;
	RefreshPoiseBar();
}

void UWGasBossBarWidget::HandleMaxPoiseChanged(float NewValue)
{
	CurrentMaxPoise = FMath::Max(NewValue, 1.f);
	RefreshPoiseBar();
}

