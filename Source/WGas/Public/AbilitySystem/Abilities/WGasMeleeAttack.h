// Fill out your copyright notice in the Description page of Project Settings.



#pragma once



#include "CoreMinimal.h"

#include "AbilitySystem/Abilities/WGasGameplayAbility.h"

#include "WGasMeleeAttack.generated.h"



class UAbilityTask_PlayMontageAndWait;

class UAnimMontage;



UCLASS(Blueprintable)

class WGAS_API UWGasMeleeAttack : public UWGasGameplayAbility

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

	virtual void PerformMeleeHitCheck();


	UFUNCTION(BlueprintCallable, Category = "Melee")

	void EndMeleeAttack(bool bWasCancelled = false);

	virtual void OnMeleeMontageFinished(bool bWasCancelled);



private:

	void ApplyAttackingTags() const;

	void RemoveAttackingTags() const;



};

