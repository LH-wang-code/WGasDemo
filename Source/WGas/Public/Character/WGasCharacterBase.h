// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "WGasCombatComponent.h"
#include "GameFramework/Character.h"
#include "Interaction/CombatInterface.h"
#include "WGasCharacterBase.generated.h"


class USkeletalMeshComponent;
class UAbilitySystemComponent;

UCLASS()
class WGAS_API AWGasCharacterBase : public ACharacter,public IAbilitySystemInterface,public ICombatInterface
{
	GENERATED_BODY()

public:

	AWGasCharacterBase();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent()const override;
	UAttributeSet* GetAttributeSet()const { return AttributeSet; }
	UWGasCombatComponent* GetCombatComponent() const { return CombatComponent; }

	/* ICombatInerface Begin*/
	virtual UAbilitySystemComponent* GetDamageableASC() const override;
	virtual bool IsAliveForCombat() const override;

	/* ICombatInerface End*/
	
protected:

	virtual void BeginPlay() override;
	//初始化角色信息
	virtual void InitAbilityActorInfo();
	void AddCharacterAbilities();

	virtual void InitializeDefaultAttributes()const;
	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect>GameplayEffectClass,float Level)const;
	
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent>AbilitySystemComponent;
	
	UPROPERTY()
	TObjectPtr<UAttributeSet>AttributeSet;

	//基础属性以及他们的最大值
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect>DefaultVitalAttributes;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect>DefaultSecondaryAttributes;
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UWGasCombatComponent>CombatComponent;
	UFUNCTION(BlueprintCallable, Category = "Combat|WeaponTrace")
	void BeginWeaponSweep();
	UFUNCTION(BlueprintCallable, Category = "Combat|WeaponTrace")
	void EndWeaponSweep();

	/** AnimBP 闪避动画结束（如 0.58）时调用，结束 Dodge GA */
	UFUNCTION(BlueprintCallable, Category = "Dodge")
	void EndDodgeFromAnimation();

	/** AnimBP 收盾/弹反动画结束时调用，结束 Block GA */
	UFUNCTION(BlueprintCallable, Category = "Block")
	void EndBlockFromAnimation();

	virtual USkeletalMeshComponent* GetWeaponTraceMesh() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<USkeletalMeshComponent>Weapon;

	
	//我们希望在角色开始时就赋予某些能力
	UPROPERTY(EditAnywhere,Category="Abilities")
	TArray<TSubclassOf<UGameplayAbility>>StartupAbilities;	
	
	UPROPERTY(EditAnywhere,Category="Abilities")
	TArray<TSubclassOf<UGameplayAbility>>StartupPassiveAbilities;	
	
private:
	bool bStartupAbilitiesGiven = false;
};
