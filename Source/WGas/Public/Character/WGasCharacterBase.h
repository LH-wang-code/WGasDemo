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

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent>AbilitySystemComponent;


	UPROPERTY()
	TObjectPtr<UAttributeSet>AttributeSet;


	
public:	






	 

public:

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<USkeletalMeshComponent>Weapon;
	//我们希望在角色开始时就赋予某些能力
	UPROPERTY(EditAnywhere,Category="Abilities")
	TArray<TSubclassOf<UGameplayAbility>>StartupAbilities;	

private:
	bool bStartupAbilitiesGiven = false;
};
