// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Animation/AnimTypes.h"
#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WGasGameplayAbility.h"
#include "WGasDodge.generated.h"

class AWGasCharacterBase;

UCLASS()
class WGAS_API UWGasDodge : public UWGasGameplayAbility
{
	GENERATED_BODY()

public:
	UWGasDodge();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	UFUNCTION(BlueprintCallable, Category = "Dodge")
	void EndDodgeActivePhase();

	UFUNCTION(BlueprintCallable, Category = "Dodge")
	void EndDodgeMovementPhase();

protected:
	void FinishDodge();
	void ApplyDodgeMovementBoost(AWGasCharacterBase* Character, const FVector& DodgeDir);
	void RestoreDodgeMovementBoost(AWGasCharacterBase* Character);
	void EnableDodgeRootMotion(AWGasCharacterBase* Character);
	void RestoreDodgeRootMotion(AWGasCharacterBase* Character);
	void SetDodgeAnimationStateOnCharacter(AWGasCharacterBase* Character, bool bIsDodging) const;

	UFUNCTION()
	void TickDodgeMovement();

private:
	FVector GetDodgeDirection(const AWGasCharacterBase* Character) const;

	bool bDodgeEndHandled = false;

	FTimerHandle DodgeMovementTickHandle;

	FVector CachedDodgeDirection = FVector::ZeroVector;
	float CachedMaxWalkSpeed = 0.f;
	bool bDodgeMovementBoostApplied = false;
	TEnumAsByte<ERootMotionMode::Type> CachedRootMotionMode = ERootMotionMode::RootMotionFromMontagesOnly;
	bool bRootMotionModeCached = false;

	/**
	 * The dodge Blend Space contains root-motion sequences. Enable extraction only
	 * while this ability is active, then restore the AnimBP's normal mode for WASD.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Dodge|Root Motion")
	bool bUseDodgeAnimationRootMotion = true;

	UPROPERTY(EditDefaultsOnly, Category = "Dodge|Movement")
	bool bBoostDodgeMovementSpeed = true;

	UPROPERTY(EditDefaultsOnly, Category = "Dodge|Movement", meta = (ClampMin = "100.0", EditCondition = "bBoostDodgeMovementSpeed"))
	float DodgeMaxWalkSpeed = 900.f;

	UPROPERTY(EditDefaultsOnly, Category = "Dodge|Movement", meta = (EditCondition = "bBoostDodgeMovementSpeed"))
	bool bApplyDodgeDirectionInput = true;

	UPROPERTY(EditDefaultsOnly, Category = "Dodge|Movement")
	bool bStopVelocityOnDodgeEnd = true;
};
