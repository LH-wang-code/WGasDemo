// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "WGasAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class WGAS_API UWGasAssetManager : public UAssetManager
{
	GENERATED_BODY()
public:
	static UWGasAssetManager& Get();


protected:
	virtual void StartInitialLoading()override;
	
	
};
