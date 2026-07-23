// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/EnemyComponent/BossGreatswordReactionComponent.h"

UBossGreatswordReactionComponent::UBossGreatswordReactionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBossGreatswordReactionComponent::RegisterGuardedHit()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	const float CurrentTime = World->GetTimeSeconds();
	if (CurrentTime - LastGuardedHitTime < GuardHitDebounceSeconds)
	{
		return;
	}
	LastGuardedHitTime = CurrentTime;
	bReleasePending = true;
}

void UBossGreatswordReactionComponent::NotifyNormalSkillFinished()
{
	if (!bReleasePending || bReleaseReady)
	{
		return;
	}
	bReleasePending = false;

	if (FMath::FRand() <= ReleaseChance)
	{
		bReleaseReady = true;
	}
}
void UBossGreatswordReactionComponent::ConsumeRelease()
{
	bReleaseReady = false;
	bReleasePending = false;
}