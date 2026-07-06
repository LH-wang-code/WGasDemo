#include "UI/Widgets/WGasStaminaBarWidget.h"

#include "Components/ProgressBar.h"
#include "UI/WidgetController/StaminaBarWGasWidgetController.h"

void UWGasStaminaBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (StaminaBar)
	{
		StaminaBar->SetBarFillType(EProgressBarFillType::BottomToTop);
	}
}

void UWGasStaminaBarWidget::SetWidgetController(UObject* InWidgetController)
{
	Super::SetWidgetController(InWidgetController);

	UStaminaBarWGasWidgetController* StaminaController = Cast<UStaminaBarWGasWidgetController>(WidgetController);
	if (!StaminaController)
	{
		return;
	}

	StaminaController->OnStaminaChanged.AddDynamic(this, &UWGasStaminaBarWidget::HandleStaminaChanged);
	StaminaController->OnMaxStaminaChanged.AddDynamic(this, &UWGasStaminaBarWidget::HandleMaxStaminaChanged);
}

void UWGasStaminaBarWidget::SetBarPercent(float CurrentValue, float MaxValue)
{
	CurrentStamina = CurrentValue;
	MaxStamina = FMath::Max(MaxValue, 1.f);
	RefreshBar();
}

void UWGasStaminaBarWidget::HandleStaminaChanged(float NewValue)
{
	CurrentStamina = NewValue;
	RefreshBar();
}

void UWGasStaminaBarWidget::HandleMaxStaminaChanged(float NewValue)
{
	MaxStamina = FMath::Max(NewValue, 1.f);
	RefreshBar();
}

void UWGasStaminaBarWidget::RefreshBar()
{
	if (!StaminaBar)
	{
		return;
	}

	const float Percent = FMath::Clamp(CurrentStamina / MaxStamina, 0.f, 1.f);
	StaminaBar->SetPercent(Percent);
}
