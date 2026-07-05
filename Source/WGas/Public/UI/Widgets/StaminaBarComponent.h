#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "StaminaBarComponent.generated.h"

UCLASS(ClassGroup = (UI), meta = (BlueprintSpawnableComponent))
class WGAS_API UStaminaBarComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UStaminaBarComponent();
};
