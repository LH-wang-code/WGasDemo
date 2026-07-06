// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "WGasCharacterBase.generated.h"


class USkeletalMeshComponent;
class UAbilitySystemComponent;

UCLASS()
class WGAS_API AWGasCharacterBase : public ACharacter,public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	AWGasCharacterBase();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent()const override;
	UAttributeSet* GetAttributeSet()const { return AttributeSet; }

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

	UPROPERTY(EditDefaultsOnly, Category = "Combat|WeaponTrace")
	FName WeaponTipSocket = TEXT("Weapon_Tip");
	UPROPERTY(EditDefaultsOnly, Category = "Combat|WeaponTrace")
	FName WeaponBaseSocket = TEXT("Weapon_Base");
	UPROPERTY(EditDefaultsOnly, Category = "Combat|WeaponTrace")
	float WeaponSweepRadius = 25.f;
	UPROPERTY(EditDefaultsOnly, Category = "Combat|WeaponTrace", meta = (ClampMin = "1", ClampMax = "16"))
	int32 WeaponSweepSampleCount = 10;
	UPROPERTY(EditDefaultsOnly, Category = "Combat|WeaponTrace")
	TSubclassOf<UGameplayEffect> MeleeDamageEffect;
	UFUNCTION(BlueprintCallable, Category = "Combat|WeaponTrace")
	void BeginWeaponSweep();
	UFUNCTION(BlueprintCallable, Category = "Combat|WeaponTrace")
	void TickWeaponSweep();
	UFUNCTION(BlueprintCallable, Category = "Combat|WeaponTrace")
	void EndWeaponSweep();




	 

public:

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<USkeletalMeshComponent>Weapon;
	//我们希望在角色开始时就赋予某些能力
	UPROPERTY(EditAnywhere,Category="Abilities")
	TArray<TSubclassOf<UGameplayAbility>>StartupAbilities;	


	UPROPERTY(EditAnywhere,Category="Abilities")
	TArray<TSubclassOf<UGameplayAbility>>StartupPassiveAbilities;	
	USkeletalMeshComponent* GetWeaponTraceMesh() const;
	
private:
	bool bStartupAbilitiesGiven = false;


	FVector LastWeaponTipPos = FVector::ZeroVector;
	FVector LastWeaponBasePos = FVector::ZeroVector;

	bool bWeaponSweepActive = false;
	TSet<TWeakObjectPtr<AActor>> MeleeHitActorsThisSwing;
	void ApplyMeleeDamageToActor(AActor* HitActor);
};
