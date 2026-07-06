#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/WGasUserWidget.h"
#include "WGasStaminaBarWidget.generated.h"

class UProgressBar;

UCLASS()
class WGAS_API UWGasStaminaBarWidget : public UWGasUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void SetWidgetController(UObject* InWidgetController) override;

	UFUNCTION(BlueprintCallable, Category = "StaminaBar")
	void SetBarPercent(float CurrentValue, float MaxValue);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> StaminaBar;

private:
	UFUNCTION()
	void HandleStaminaChanged(float NewValue);

	UFUNCTION()
	void HandleMaxStaminaChanged(float NewValue);

	void RefreshBar();

	float CurrentStamina = 0.f;
	float MaxStamina = 1.f;
};
