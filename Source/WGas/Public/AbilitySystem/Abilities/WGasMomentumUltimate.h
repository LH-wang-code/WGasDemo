// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WGasDamageGameplayAbility.h"
#include "WGasMomentumUltimate.generated.h"

/**
 * C++ 负责：CanActivate、Commit、消耗气势、Tag、DamageWindow。
 * 蓝图负责：OnMomentumUltimateActivated 里播 Montage，结束时调 EndMomentumUltimate。
 */
UCLASS(Blueprintable)
class WGAS_API UWGasMomentumUltimate : public UWGasDamageGameplayAbility
{
	GENERATED_BODY()

public:
	UWGasMomentumUltimate();

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

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

	/** Montage 播完 / 被打断时在蓝图里调用 */
	UFUNCTION(BlueprintCallable, Category = "Momentum")
	void EndMomentumUltimate(bool bWasCancelled = false);

	/** 大招蓝图里取锁敌目标（不经过 Component 引脚） */
	UFUNCTION(BlueprintPure, Category = "Momentum|LockOn")
	bool IsMomentumUltimateLockOnActive() const;

	UFUNCTION(BlueprintPure, Category = "Momentum|LockOn")
	AActor* GetMomentumUltimateLockTarget() const;

	UFUNCTION(BlueprintPure, Category = "Momentum|LockOn")
	FVector GetMomentumUltimateLockOnLocation() const;

	/** 更新 Motion Warp 目标（穿透：Warp 点在锁点后方）。每次刺之前调用。 */
	UFUNCTION(BlueprintCallable, Category = "Momentum|LockOn")
	bool UpdateUltimateWarpTarget();

	UFUNCTION(BlueprintCallable, Category = "Momentum|LockOn")
	void ClearUltimateWarpTarget();

protected:
	UFUNCTION(BlueprintCallable, Category = "Momentum")
	void BeginMomentumUltimate();

	/** 蓝图实现：Play Montage、锁敌、特效等 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Momentum|Animation")
	void OnMomentumUltimateActivated();

	void FinishUltimate(bool bWasCancelled);

	UPROPERTY(EditDefaultsOnly, Category = "Momentum|Movement")
	bool bStopMovementOnAttack = true;

	/** 与 Montage Motion Warping Notify 上的 Warp Target Name 一致 */
	UPROPERTY(EditDefaultsOnly, Category = "Momentum|LockOn")
	FName LockOnWarpTargetName = FName("AttackTarget");

	/** true：Warp 点在锁点后方（穿透）；false：停在锁点前（同普攻） */
	UPROPERTY(EditDefaultsOnly, Category = "Momentum|LockOn")
	bool bUsePierceWarpTarget = true;

	/** 穿透时越过锁点的距离 */
	UPROPERTY(EditDefaultsOnly, Category = "Momentum|LockOn", meta = (ClampMin = "0.0", EditCondition = "bUsePierceWarpTarget"))
	float PiercePastDistance = 250.f;

	/** 非穿透 / 无锁敌时，停在目标前的距离 */
	UPROPERTY(EditDefaultsOnly, Category = "Momentum|LockOn", meta = (ClampMin = "0.0"))
	float StrikeStopDistance = 150.f;

private:
	bool bUltimateEndHandled = false;
};
