// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "WGasEffectTypes.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;

USTRUCT(BlueprintType)
struct FDamageEffectParams
{
	GENERATED_BODY()

	FDamageEffectParams() {}

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UObject> WorldContextObject = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> DamageGameplayEffectClass = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> SourceAbilitySystemComponent;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent;

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag DamageType = FGameplayTag();
	
	UPROPERTY(BlueprintReadWrite)
	float BaseDamage = 0.f;
};

USTRUCT(BlueprintType)
struct FWGasGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

public:


	
	TSharedPtr<FGameplayTag> GetDamageType()const { return DamageType; }
	void SetDamageType(TSharedPtr<FGameplayTag>InDamageType) { DamageType = InDamageType; }

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}

protected:
	TSharedPtr<FGameplayTag>DamageType;

};
USTRUCT(BlueprintType)
struct FDamagePayLoad
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	FGameplayTag DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float BaseDamage = 0.f;
	
	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	TObjectPtr<UAbilitySystemComponent> SourceASC = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	TObjectPtr<AActor> Instigator = nullptr;

	bool IsValid() const
	{
		return DamageEffectClass != nullptr&& SourceASC != nullptr&& BaseDamage > 0.f&& DamageType.IsValid();
	}
};