// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "Components/ActorComponent.h"
#include "BossPoiseBrokenComponent.generated.h"

class AWGasCharacterEnemy;
class UBossAIPauseComponent;
class UBossPhaseTransitionComponent;
class UAnimMontage;
class UGameplayEffect;


UCLASS( ClassGroup=(Boss), meta=(BlueprintSpawnableComponent) )
class WGAS_API UBossPoiseBrokenComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBossPoiseBrokenComponent();
	void Initialize(AWGasCharacterEnemy* InOwner,UBossAIPauseComponent* InAIPause,UBossPhaseTransitionComponent* InPhase);

	void BindDelegates();
	bool IsPoiseBrokenActive() const;
	float GetIncomingDamageMultiplier() const { return PoiseBrokenIncomingDamageMultiplier; }
	/** 转阶段打断破韧，不 ResumeBrain */
	void ForceCleanupWithoutResumeBrain();
protected:

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction) override;
private:
	TWeakObjectPtr<AWGasCharacterEnemy> OwnerEnemy;
	TWeakObjectPtr<UBossAIPauseComponent> AIPause;
	TWeakObjectPtr<UBossPhaseTransitionComponent> Phase;
	UPROPERTY(EditDefaultsOnly, Category="Poise", meta=(ClampMin="1.0"))
	float PoiseBrokenIncomingDamageMultiplier = 1.5f;
	UPROPERTY(EditDefaultsOnly, Category="Poise")
	TSubclassOf<UGameplayEffect> PoiseBrokenEffectClass;
	UPROPERTY(EditDefaultsOnly, Category="Poise")
	TSubclassOf<UGameplayEffect> PoiseRegenEffectClass;
	UPROPERTY(EditDefaultsOnly, Category="Poise")
	TObjectPtr<UAnimMontage> PoiseBrokenMontage;
	UPROPERTY(EditDefaultsOnly, Category="Poise")
	FName PoiseBrokenBlackboardKey = TEXT("bPoiseBroken");
	UPROPERTY(EditDefaultsOnly, Category="Poise")
	FName PoiseBrokenEndSection = TEXT("End");

	void EnterPoiseBroken();
	void ExitPoiseBroken();
	void RequestPoiseBrokenExit();
	void CleanupWithoutResumeBrain();
	void LockRotation();
	void UnlockRotation();
	bool ShouldSkipPoise() const;

	
	UFUNCTION()
	void OnPoiseBrokenMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	FActiveGameplayEffectHandle PoiseBrokenEffectHandle;
	FActiveGameplayEffectHandle PoiseRegenEffectHandle;
	bool bDelegatesBound = false;
	bool bExitPending = false;
	bool bRotationLocked = false;
	bool bPoiseBrokenRotationLocked = false;
	bool bCachedUseControllerRotationYaw = false;
	bool bCachedUseControllerDesiredRotation = false;
	bool bCachedOrientRotationToMovement = false;
	FRotator CachedLockedRotation = FRotator::ZeroRotator;
	FRotator CachedRotationRate = FRotator::ZeroRotator;
};
