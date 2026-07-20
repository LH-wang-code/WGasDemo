// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BossAIPauseComponent.generated.h"
class AWGasCharacterEnemy;
class UBlackboardComponent;
class UBehaviorTree;
UCLASS( ClassGroup=(Boss), meta=(BlueprintSpawnableComponent) )
class WGAS_API UBossAIPauseComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBossAIPauseComponent();
	void Initialize(AWGasCharacterEnemy* InOwner, UBehaviorTree* InBehaviorTree);
	
	void CacheDefaultMovement();
	void StopMovement();
	void CleanupPathAndFocus();
	void ResetDefaultMovement();

	bool PauseBrain(FName Reason);
	void ResumeBrain(FName Reason, bool bRestartBehaviorTree = true);
	bool IsBrainPaused(FName Reason) const;

	
	void DisableMovement();          // 破韧用
	void RestoreMovement();          // 解锁移动 + RootMotion

	void SetBlackboardBool(FName Key, bool bValue);
	void SetBlackboardInt(FName Key, int32 Value);
	UBlackboardComponent* GetBlackboard() const;
	void ClearPauseReason(FName Reason); 
private:
	TWeakObjectPtr<AWGasCharacterEnemy> OwnerEnemy;
	TWeakObjectPtr<UBehaviorTree> BehaviorTree;
	FName ActiveBrainPauseReason = NAME_None;
	// Movement 默认值缓存
	bool bDefaultUseControllerRotationYaw = false;
	bool bDefaultUseControllerDesiredRotation = false;
	bool bDefaultOrientRotationToMovement = false;
	FRotator DefaultRotationRate = FRotator::ZeroRotator;
	float CachedDefaultMaxWalkSpeed = 600.f;
	// DisableMovement 缓存
	bool bMovementDisabled = false;
	TEnumAsByte<EMovementMode> CachedMovementMode = MOVE_Walking;
	TEnumAsByte<ERootMotionMode::Type> CachedRootMotionMode = ERootMotionMode::RootMotionFromMontagesOnly;
	bool bRootMotionModeCached = false;
		
};
