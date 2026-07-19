// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WGasGameplayAbility.h"
#include "ActiveGameplayEffectHandle.h"
#include "WGasBlock.generated.h"

class AWGasCharacterBase;
class UAbilitySystemComponent;
class UGameplayEffect;

UCLASS()
class WGAS_API UWGasBlock : public UWGasGameplayAbility
{
	GENERATED_BODY()

public:
	UWGasBlock();

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

	virtual void InputReleased(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

	/** AnimBP 收盾/弹刀动画结束时调用 */
	void EndBlockActivePhase();

	static UWGasBlock* GetActiveBlock(UAbilitySystemComponent* ASC);

	/** 格挡中按 LMB：播弹反动画，等 AnimBP 调 EndBlockActivePhase */
	void TryParryFromAttackInput();
	static float GetBlockDamageMultiplierForTarget(UAbilitySystemComponent* ASC);
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Block|Movement", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float BlockMoveSpeedMultiplier = 0.45f;

	UPROPERTY(EditDefaultsOnly, Category = "Block|Cost")
	TSubclassOf<UGameplayEffect> BlockStaminaDrainEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Block|Defense", meta = (ClampMin = "0", ClampMax = "1"))
	float BlockDamageMultiplier = 0.3f;
private:
	
	bool bBlockEndHandled = false;
	bool bParryTriggered = false;
	float CachedMaxWalkSpeed = 0.f;
	bool bBlockMovementPenaltyApplied = false;
	FActiveGameplayEffectHandle BlockStaminaDrainEffectHandle;

	void SetBlockAnimationStateOnCharacter(AWGasCharacterBase* Character, bool bIsBlocking) const;
	void SetParryAnimationStateOnCharacter(AWGasCharacterBase* Character, bool bIsParrying) const;
	void ApplyBlockMovementPenalty(AWGasCharacterBase* Character);
	void RestoreBlockMovementPenalty(AWGasCharacterBase* Character);
	void RemoveBlockStaminaDrainEffect();
	void FinishBlock();
};
