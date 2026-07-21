// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/WGasCharacterBase.h"
#include "Input/GasInputBufferComponent.h"
#include "WGasCharacterHero.generated.h"

/**
 * 
 */

class UCameraComponent;
 
class USpringArmComponent;
class UGasInputBufferComponent;
class UWGasLockOnComponent;
class UMotionWarpingComponent;

UCLASS()
class WGAS_API AWGasCharacterHero : public AWGasCharacterBase
{
	GENERATED_BODY()
public:
	AWGasCharacterHero();
	virtual void PossessedBy(AController* NewController)override;

	
	UFUNCTION(BlueprintPure,Category="Input")
	UGasInputBufferComponent*GetWGasInputBufferComponent()const {return InputBufferComponent;}

	UFUNCTION(BlueprintPure, Category = "Combat")
	UWGasLockOnComponent* GetLockOnComponent() const { return LockOnComponent; }

	/** 蓝图便捷接口：不直接从 Component 引脚调方法时用这些 */
	UFUNCTION(BlueprintPure, Category = "Combat|LockOn")
	bool IsLockOnActive() const;

	UFUNCTION(BlueprintPure, Category = "Combat|LockOn")
	AActor* GetLockOnTargetActor() const;

	UFUNCTION(BlueprintPure, Category = "Combat|LockOn")
	FVector GetLockOnTargetLocation() const;

	UFUNCTION(BlueprintPure, Category = "Motion Warping")
	UMotionWarpingComponent* GetMotionWarpingComponent() const { return MotionWarpingComponent; }
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void ToggleWalkRun();

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsRunning() const { return bIsRunning; }
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void ForceWalk();

	void UpdateRunningTag(const FVector2D& MoveInput);
	void ClearRunningTag();

	/** Boss 命中无敌帧时触发，蓝图里 Spawn PoseableMesh 残影 */
	UFUNCTION(BlueprintNativeEvent, Category = "Combat|Dodge")
	void OnDodgeIFrameSuccess();

	//气势
	UFUNCTION(BlueprintCallable, Category = "Momentum")
	void GrantMomentum(float Amount);
	UFUNCTION(BlueprintCallable, Category = "Momentum")
	void NotifyParrySuccess();

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Hit")
	TObjectPtr<UAnimMontage> HitReactMontage;
	
	UFUNCTION(BlueprintCallable, Category = "Combat|Hit")
	void NotifyHitReact(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Combat|Hit")
	void EndHitReactFromAnimation();

	void OnHitReactMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
protected:

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float WalkSpeed = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float RunSpeed = 600.f;

	UPROPERTY(EditDefaultsOnly, Category = "Momentum")
	float MomentumGainOnPerfectDodge = 34.f;
	UPROPERTY(EditDefaultsOnly, Category = "Momentum")
	float MomentumGainOnParrySuccess = 34.f;
	void ApplyMovementSpeed();

	virtual void HandleDeathExtras() override;
	
	TObjectPtr<UGasInputBufferComponent>InputBufferComponent;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> StaminaCostGE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Warping")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UWGasLockOnComponent> LockOnComponent;
	
private:
	void BindStaminaDepletedDelegate();
	bool bStaminaDepletedBound = false;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent>CameraComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent>SpringArmComponent;
	
	bool bIsRunning = false;
	bool bRunStaminaEffectsApplied = false;
	virtual void InitAbilityActorInfo() override;
};
