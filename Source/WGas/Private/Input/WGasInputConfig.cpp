// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/WGasInputConfig.h"

const UInputAction* UWGasInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag,
	bool bLogNotFound) const
{

	for (const FWGasInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction&& Action.InputTag == InputTag)
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp,Error,TEXT("%s"),TEXT("Input Action Not Found"));
	}
	return nullptr;
}
