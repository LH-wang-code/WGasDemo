// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "BossAttackInfo.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FBossAttackInfomation
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag AbilityTag;

	UPROPERTY(EditDefaultsOnly)
	float MinRange=0.f;

	UPROPERTY(EditDefaultsOnly)
	float MaxRange=300.f;

	UPROPERTY(EditDefaultsOnly)
	float Weight=1.f;
	
};
UCLASS()
class WGAS_API UBossAttackInfo : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	TArray<FBossAttackInfomation>Attacks;
	
};
