// Fill out your copyright notice in the Description page of Project Settings.


#include "WGasAssetManager.h"

#include "WGasGameplayTags.h"

UWGasAssetManager& UWGasAssetManager::Get()
{
	check(GEngine);
	UWGasAssetManager* WGasAssetManager=Cast<UWGasAssetManager>(GEngine->AssetManager);
	return *WGasAssetManager;
}

void UWGasAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	//通过assetmamager管理tag，方便后续使用
	FWGasGameplayTags::InitializeNativeGameplayTags();
}
