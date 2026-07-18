// Fill out your copyright notice in the Description page of Project Settings.



#pragma once



#include "CoreMinimal.h"
#include "WGasDamageGameplayAbility.h"

#include "AbilitySystem/Abilities/WGasGameplayAbility.h"

#include "WGasMeleeAttack.generated.h"



class UAbilityTask_PlayMontageAndWait;

class UAnimMontage;
class UAbilitySystemComponent;


UCLASS(Blueprintable)

class WGAS_API UWGasMeleeAttack : public UWGasDamageGameplayAbility

{

	GENERATED_BODY()



public:

	UWGasMeleeAttack();

	virtual void InputPressed(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void InputReleased(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	/** Ability 激活期间再次按下攻击键时触发，蓝图里 Set Combo Queued = true */
	UFUNCTION(BlueprintImplementableEvent, Category = "Melee|Combo")
	void OnComboInputReceived();

	UFUNCTION(BlueprintCallable, Category = "Melee|Phase")
	bool TryCancelFromRecovery();
	/** 后摇阶段移动/跑步：淡出攻击 Montage，切回 Locomotion */
	UFUNCTION(BlueprintCallable, Category = "Melee|Phase")
	bool CancelAttackForLocomotion(float BlendOutTime = -1.f);
	//前后摇
	UFUNCTION(BlueprintCallable, Category = "Melee|Phase")
	bool IsInAttackLighting() const;
	UFUNCTION(BlueprintCallable, Category = "Melee|Phase")
	bool TryCancelFromCancelablePhase();
	static UWGasMeleeAttack* GetActiveMeleeAttack(UAbilitySystemComponent* ASC);
	static bool HasAnyAttackPhaseTag(const UAbilitySystemComponent* ASC);
	/** GA 仍 Active 但无阶段 Tag 且未播 Montage，视为异常卡住 */
	static bool IsStaleActiveAttack(UAbilitySystemComponent* ASC);
	static void ClearAttackPhaseTags(UAbilitySystemComponent* ASC);
	/** Montage 已结束但仍有攻击 Tag / GA 未关时强制清理（移动输入等场景调用） */
	static void SanitizeStaleAttackState(UAbilitySystemComponent* ASC);

	UFUNCTION(BlueprintCallable, Category = "Melee")
	void EndMeleeAttack(bool bWasCancelled = false);

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Tags")

	FGameplayTag AttackingStateTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Damage")
	TArray<float>AttackingDamage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Melee|Damage")
	int32 ComboIdx=0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Movement")
	bool bStopMovementOnAttack = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Movement")
	float RecoveryLocomotionBlendOutTime = 0.15f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Hit")

	float MeleeTraceDistance = 150.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Hit")

	float MeleeTraceRadius = 40.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|LightAttack")

	TArray<TObjectPtr<UAnimMontage>> LightAttackMontages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|LockOn")
	float LockOnAttackDistance = 150.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|LockOn")
	FName LockOnWarpTargetName = FName("AttackTarget");
	
	UFUNCTION(BlueprintCallable, Category = "Melee")

	void BeginMeleeAttack();

	/** 锁敌时更新 Motion Warp 目标（每段 Montage 播前都应调用） */
	UFUNCTION(BlueprintCallable, Category = "Melee|LockOn")
	bool UpdateLockOnWarpTarget();

	UFUNCTION(BlueprintCallable, Category = "Melee|Phase")
	void EnterAttackReovery();
	/** 后摇段按下攻击：取消当前段，蓝图接下一段 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Melee|Combo")
	void OnRecoveryCancelIntoCombo();
	UFUNCTION(BlueprintCallable, Category = "Melee|Phase")
	bool IsInAttackActive() const;
	UFUNCTION(BlueprintCallable, Category = "Melee|Phase")
	bool IsInAttackRecovery() const;
	/** 后摇阶段取消（闪避/格挡/下一段攻击前调用） */

	
	virtual void OnMeleeMontageFinished(bool bWasCancelled);


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Tags")
	FGameplayTag AttackingActiveTag;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Tags")
	FGameplayTag AttackingRecoveryTag;
private:
	

	void RemoveAttackingTags() const;

	void EnterAttackActive();
	void RemoveAllAttackingTags() const;
	void StopCurrentAttackMontage(float BlendOutTime = 0.1f) const;
	void ClearLockOnWarpTarget() const;
	void RestoreAttackMovementState() const;
	static bool IsAttackMontagePlaying(const UAbilitySystemComponent* ASC);
};
