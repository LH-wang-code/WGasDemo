// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "WGasInputConfig.h"
#include "WGasInputComponent.generated.h"

/**
 * 用于技能按键的批量绑定
 */
UCLASS()
class WGAS_API UWGasInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()
public:
	template<class UserClass,typename PressedFuncType,typename ReleasedFuncType,typename HeldFuncType>
	void BindAbilityActions(
		const UWGasInputConfig* InputConfig,
		UserClass* Object,
		PressedFuncType PressedFunc,
		ReleasedFuncType ReleasedFunc,
		HeldFuncType HeldFunc)
	{
		check(InputConfig);
		for (const FWGasInputAction& Action:InputConfig->AbilityInputActions)
		{
			if (Action.InputAction&&Action.InputTag.IsValid())
			{
				if (HeldFunc)
				{
					BindAction(Action.InputAction,ETriggerEvent::Triggered,Object,HeldFunc,Action.InputTag);
				}
				
				if (PressedFunc)
				{
					BindAction(Action.InputAction,ETriggerEvent::Started,Object,PressedFunc,Action.InputTag);
				}
				if (ReleasedFunc)
				{
					BindAction(Action.InputAction,ETriggerEvent::Completed,Object,ReleasedFunc,Action.InputTag);
				}
			}
		}
	}
};
