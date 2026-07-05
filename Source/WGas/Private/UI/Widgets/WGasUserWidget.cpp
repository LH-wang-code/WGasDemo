// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/WGasUserWidget.h"

void UWGasUserWidget::SetWidgetController(UObject* InWidgetController)
{
	WidgetController=InWidgetController;
	WidgetControllerSet();

}
