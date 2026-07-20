// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WGasParriedComponent.generated.h"

class AWGasCharacterBase;
class UAnimMontage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnParriedEvent, AActor*, ParryInstigator);

UCLASS( ClassGroup=(Combat), meta=(BlueprintSpawnableComponent) )
class WGAS_API UWGasParriedComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	/**
	 * 
	 */
	UWGasParriedComponent();
	UFUNCTION(BlueprintCallable, Category = "Parried")
	bool EnterParried(AActor* ParryInstigator = nullptr);
	UFUNCTION(BlueprintCallable, Category = "Parried")
	void ExitParried();

	UFUNCTION(BlueprintCallable, Category = "Parried")
	void ForceExitParried();

	UFUNCTION(BlueprintPure, Category = "Parried")
	bool IsParriedActive() const { return bParriedActive; }

	UPROPERTY(BlueprintAssignable, Category = "Parried")
	FOnParriedEvent OnParriedStarted;

	UPROPERTY(BlueprintAssignable, Category = "Parried")
	FOnParriedEvent OnParriedEnded;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Parried")
	bool bCanBeParried = true;
	
	UPROPERTY(EditDefaultsOnly, Category = "Parried")
	TObjectPtr<UAnimMontage> ParriedMontage;
	
	/** 未配置蒙太奇 / 播放失败时的硬直时长 */
	UPROPERTY(EditDefaultsOnly, Category = "Parried", meta = (ClampMin = "0.05"))
	float FallbackParriedDuration = 0.45f;

	/** 硬直蒙太奇播完后的原地发呆时长（秒），再恢复 AI */
	UPROPERTY(EditDefaultsOnly, Category = "Parried", meta = (ClampMin = "0"))
	float PostParriedIdleDuration = 0.8f;
	
	/** Boss 行为树黑板键（有 BossAIPause 时才写） */
	UPROPERTY(EditDefaultsOnly, Category = "Parried|AI")
	FName ParriedBlackboardKey = TEXT("bParried");
	
	UPROPERTY(EditDefaultsOnly, Category = "Parried|AI")
	FName BrainPauseReason = TEXT("Parried");

	virtual void BeginPlay() override;
private:
	void SuspendActions();
	void ResumeActions();
	bool ShouldSkipParried() const;

	void ApplyParriedTag();
	void RemoveParriedTag();
	void CancelActiveAttacks();
	void PlayParriedMontage();
	void StartPostParriedIdleTimer();
	void ClearFallbackTimer();

	UFUNCTION()
	void OnParriedMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	TWeakObjectPtr<AWGasCharacterBase> OwnerCharacter;
	FTimerHandle FallbackTimerHandle;
	bool bParriedActive = false;
	bool bAppliedMovementLock = false;
	bool bBossBrainPaused = false;
	bool bBossMovementDisabled = false;
	bool bAppliedDirectBrainStop = false;
};
