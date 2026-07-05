#include "UI/WidgetController/StaminaBarWGasWidgetController.h"

#include "AbilitySystem/WGasAttributeSet.h"

void UStaminaBarWGasWidgetController::BroadcastInitialValues()
{
	if (!GetWGasAS())
	{
		return;
	}

	OnStaminaChanged.Broadcast(GetWGasAS()->GetStamina());
	OnMaxStaminaChanged.Broadcast(GetWGasAS()->GetMaxStamina());
}

void UStaminaBarWGasWidgetController::BindCallbacksToDependencies()
{
	if (!AbilitySystemComponent || !GetWGasAS())
	{
		return;
	}

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetWGasAS()->GetStaminaAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnStaminaChanged.Broadcast(Data.NewValue);
		});

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetWGasAS()->GetMaxStaminaAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnMaxStaminaChanged.Broadcast(Data.NewValue);
		});
}
