// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WGasAnimLayerInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UWGasAnimLayerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * AnimBP 实现此接口：开始/结束闪避时同步 bIsDodging。
 * 结束时机与方向由 AnimBP 蓝图处理。
 */
class WGAS_API IWGasAnimLayerInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dodge|Animation")
	void SetDodgeAnimationState(bool bIsDodging);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Block|Animation")
	void SetBlockAnimationState(bool bIsBlocking);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Block|Animation")
	void SetParryAnimationState(bool bIsParrying);
};
