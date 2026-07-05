// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"

#include "WGasGamemode.generated.h"
/**
 * 
 */
UCLASS()
class WGAS_API AWGasGamemode : public AGameModeBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly,Category="Default Character Class")
	TObjectPtr<UCharacterClassInfo> CharacterClassInfo;
};
