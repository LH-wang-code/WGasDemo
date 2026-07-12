// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Character/WGasCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "WGasCharacterEnemy.generated.h"

class UBossAttackInfo;
class UAnimMontage;
class UBlackboardComponent;
/**
 * 
 */
class AWGasAIController;
class UBehaviorTree;
class UMotionWarpingComponent;

UCLASS()
class WGAS_API AWGasCharacterEnemy : public AWGasCharacterBase
{
	GENERATED_BODY()

public:
	AWGasCharacterEnemy();

	virtual void PossessedBy(AController* NewController)override;

	UMotionWarpingComponent* GetMotionWarpingComponent() const { return MotionWarpingComponent; }
	UPROPERTY(EditDefaultsOnly, Category = "AI|Combat")
	TObjectPtr<UBossAttackInfo> AttackSet;

protected:
	ECharacterClass CharacterClass=ECharacterClass::Warrior;
	int32 Level=1;

	//AIController
	UPROPERTY(EditAnywhere,Category="AI")
	TObjectPtr<UBehaviorTree>BehaviorTree;

	UPROPERTY(EditAnywhere, Category = "AI")
	TObjectPtr<AWGasAIController>WGasAIController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Warping")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;


	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	virtual void InitAbilityActorInfo() override;
	virtual void InitializeDefaultAttributes()const override;

	virtual USkeletalMeshComponent* GetWeaponTraceMesh() const override;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Poise")
	TSubclassOf<UGameplayEffect> PoiseBrokenEffectClass;
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Poise")
	TSubclassOf<UGameplayEffect> PoiseRegenEffectClass;
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Poise")
	TObjectPtr<UAnimMontage> PoiseBrokenMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Poise")
	FName PoiseBrokenBlackboardKey = TEXT("bPoiseBroken");

	void EnterPoiseBroken();
	void ExitPoiseBroken();
	void BindPoiseBrokenDelegates();
	UBlackboardComponent* GetBossBlackboard();
	void SetPoiseBrokenBlackboard(bool bBroken);
	void StopBossMovement();
	void LockBossMovementForPoiseBroken();
	void UnlockBossMovementForPoiseBroken();
	void PauseBossBrainForPoiseBroken();
	void ResumeBossBrainForPoiseBroken();
	void LockBossRotationForPoiseBroken();
	void UnlockBossRotationForPoiseBroken();

	FActiveGameplayEffectHandle PoiseBrokenEffectHandle;
	FActiveGameplayEffectHandle PoiseRegenEffectHandle;
	bool bPoiseBrokenDelegatesBound = false;
	bool bMovementLockedForPoise = false;
	bool bBrainPausedForPoise = false;
	bool bRotationLockedForPoise = false;
	bool bPoiseBrokenRotationLocked = false;
	bool bCachedUseControllerRotationYawForPoise = false;
	bool bCachedUseControllerDesiredRotationForPoise = false;
	bool bCachedOrientRotationToMovementForPoise = false;
	FRotator CachedPoiseBrokenRotation = FRotator::ZeroRotator;
	FRotator CachedRotationRateForPoise = FRotator::ZeroRotator;
	TEnumAsByte<EMovementMode> CachedMovementMode = MOVE_Walking;
	float CachedDefaultMaxWalkSpeed = 600.f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Poise")
	FName PoiseBrokenEndSection = TEXT("End");
	void RequestPoiseBrokenExit();
	UFUNCTION()
	void OnPoiseBrokenMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	bool bPoiseExitPending = false;
};
