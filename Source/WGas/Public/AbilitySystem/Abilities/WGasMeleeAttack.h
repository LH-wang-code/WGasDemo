// Fill out your copyright notice in the Description page of Project Settings.



#pragma once



#include "CoreMinimal.h"
#include "WGasDamageGameplayAbility.h"

#include "AbilitySystem/Abilities/WGasGameplayAbility.h"

#include "WGasMeleeAttack.generated.h"



class UAbilityTask_PlayMontageAndWait;

class UAnimMontage;



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

	/** Ability 激活期间再次按下攻击键时触发，蓝图里 Set Combo Queued = true */
	UFUNCTION(BlueprintImplementableEvent, Category = "Melee|Combo")
	void OnComboInputReceived();

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Tags")

	FGameplayTag AttackingStateTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Damage")
	TArray<float>AttackingDamage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Melee|Damage")
	int32 ComboIdx=0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Movement")

	bool bStopMovementOnAttack = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Hit")

	float MeleeTraceDistance = 150.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Hit")

	float MeleeTraceRadius = 40.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|LightAttack")

	TArray<TObjectPtr<UAnimMontage>> LightAttackMontages;
	
	UFUNCTION(BlueprintCallable, Category = "Melee")

	void BeginMeleeAttack();


	UFUNCTION(BlueprintCallable, Category = "Melee")

	void EndMeleeAttack(bool bWasCancelled = false);
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
	UFUNCTION(BlueprintCallable, Category = "Melee|Phase")
	bool TryCancelFromRecovery();
	
	virtual void OnMeleeMontageFinished(bool bWasCancelled);


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Tags")
	FGameplayTag AttackingActiveTag;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Tags")
	FGameplayTag AttackingRecoveryTag;
private:

	void ApplyAttackingTags() const;

	void RemoveAttackingTags() const;

	void EnterAttackActive();
	void RemoveAllAttackingTags() const;
	void StopCurrentAttackMontage(float BlendOutTime = 0.1f) const;

};
