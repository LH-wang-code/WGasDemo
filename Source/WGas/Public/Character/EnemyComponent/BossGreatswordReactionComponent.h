// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BossGreatswordReactionComponent.generated.h"


UCLASS( ClassGroup=(Boss), meta=(BlueprintSpawnableComponent) )
class WGAS_API UBossGreatswordReactionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBossGreatswordReactionComponent();
	UFUNCTION(BlueprintCallable, Category="Boss|Greatsword Guard")
	void RegisterGuardedHit();
	UFUNCTION(BlueprintCallable, Category="Boss|Greatsword Guard")
	void NotifyNormalSkillFinished();
	UFUNCTION(BlueprintPure, Category="Boss|Greatsword Guard")
	bool IsReleaseReady() const { return bReleaseReady; }
	UFUNCTION(BlueprintCallable, Category="Boss|Greatsword Guard")
	void ConsumeRelease();
protected:
	UPROPERTY(EditDefaultsOnly, Category="Boss|Greatsword Guard",meta=(ClampMin="0.0", ClampMax="1.0"))
	float ReleaseChance = 1.0f;

	// 防止同一次武器 Sweep 多次命中而重复触发。
	UPROPERTY(EditDefaultsOnly, Category="Boss|Greatsword Guard",meta=(ClampMin="0.0"))
	float GuardHitDebounceSeconds = 0.2f;
private:
	bool bReleasePending = false;
	bool bReleaseReady = false;
	float LastGuardedHitTime = -1000.f;
		
};
