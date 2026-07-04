// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WGasGameplayAbility.h"
#include "WGasDodge.generated.h"

class AWGasCharacterBase;
class UAnimMontage;
UCLASS()
class WGAS_API UWGasDodge : public UWGasGameplayAbility
{
	GENERATED_BODY()
public:
	UWGasDodge();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION()
	void OnDodgeMontageEnded();
protected:
	void PerformDodge();

private:
	FVector GetDodgeDirection(const AWGasCharacterBase* Character) const;
	UAnimMontage* GetDodgeMontageForDirection(const FVector& DodgeDir, const AWGasCharacterBase* Character) const;

	/** 防止 OnCompleted / OnInterrupted 重复 End */
	bool bDodgeEndHandled = false;

public:
	/** 根运动 Montage 位移时请保持 false，避免和 LaunchCharacter 叠加 */
	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	bool bUseLaunchImpulse = false;

	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	float DodgeLaunchPower = 1200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Dodge|8-Way")
	TObjectPtr<UAnimMontage> DodgeForwardMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Dodge|8-Way")
	TObjectPtr<UAnimMontage> DodgeForwardRightMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Dodge|8-Way")
	TObjectPtr<UAnimMontage> DodgeRightMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Dodge|8-Way")
	TObjectPtr<UAnimMontage> DodgeBackRightMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Dodge|8-Way")
	TObjectPtr<UAnimMontage> DodgeBackMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Dodge|8-Way")
	TObjectPtr<UAnimMontage> DodgeBackLeftMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Dodge|8-Way")
	TObjectPtr<UAnimMontage> DodgeLeftMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Dodge|8-Way")
	TObjectPtr<UAnimMontage> DodgeForwardLeftMontage;
};


