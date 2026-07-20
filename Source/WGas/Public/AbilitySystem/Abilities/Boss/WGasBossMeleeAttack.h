// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WGasDamageGameplayAbility.h"
#include "WGasBossMeleeAttack.generated.h"

/**
 * 
 */
UCLASS()
class WGAS_API UWGasBossMeleeAttack : public UWGasDamageGameplayAbility
{
	GENERATED_BODY()
public:
	UWGasBossMeleeAttack();


	virtual void ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData) override;
	UFUNCTION(BlueprintCallable, Category = "Boss|Melee")
	void BeginBossMeleeAttack();
	UFUNCTION(BlueprintCallable, Category = "Boss|Melee")
	void EndBossMeleeAttack(bool bWasCancelled = false);

	static UWGasBossMeleeAttack* GetActiveBossMeleeAttack(UAbilitySystemComponent* ASC);
	static bool TryInterruptFromParry(UAbilitySystemComponent* AttackerASC);
	bool TryInterruptFromParry();
protected:

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Melee")
	TObjectPtr<UAnimMontage> AttackMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Melee")
	bool bStopMovementOnAttack = true;
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Melee")
	bool bFaceTargetOnAttack = true;

	void FaceTargetActor();
	void ApplyAttackingTags() const;
	void RemoveAttackingTags() const;
	virtual void FinishAttack(bool bWasCancelled);
	UFUNCTION()
	void OnAttackMontageEnded();
	bool bAttackEndHandled = false;
	bool bCachedOrientRotationToMovement = false;
	bool bCachedUseControllerDesiredRotation = false;
	bool bCachedAllowPhysicsRotationDuringAnimRootMotion = false;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Melee")
	bool bInterruptibleByParry = true;
	void StopAttackMontage(float BlendOutTime = 0.1f) const;
};
