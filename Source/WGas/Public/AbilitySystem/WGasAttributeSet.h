// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "WGasEffectTypes.h"
#include "WGasAttributeSet.generated.h"

/**
 * 
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
USTRUCT()
struct FEffectProperties
{
	GENERATED_BODY()

	FEffectProperties() {}

	FGameplayEffectContextHandle EffectContextHandle;

	UPROPERTY()
	UAbilitySystemComponent* SourceASC = nullptr;

	UPROPERTY()
	AActor* SourceAvatarActor = nullptr;
	UPROPERTY()

	AController* SourceController = nullptr;
	UPROPERTY()

	ACharacter* SourceCharacter = nullptr;

	UPROPERTY()
	UAbilitySystemComponent* TargetASC = nullptr;

	UPROPERTY()
	AActor* TargetAvatarActor = nullptr;
	UPROPERTY()

	AController* TargetController = nullptr;
	UPROPERTY()

	ACharacter* TargetCharacter = nullptr;

};

template<class T>
using TStaticFuncPtr = typename TBaseStaticDelegateInstance<T, FDefaultDelegateUserPolicy>::FFuncPtr;
DECLARE_MULTICAST_DELEGATE(FOnStaminaDepleted);
DECLARE_MULTICAST_DELEGATE(FOnPoiseBroken);
DECLARE_MULTICAST_DELEGATE(FOnPoiseRecovered);
DECLARE_MULTICAST_DELEGATE(FOnMomentumFilled);
UCLASS()
class WGAS_API UWGasAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UWGasAttributeSet();

	FOnStaminaDepleted OnStaminaDepleted;
	FOnPoiseBroken OnPoiseBroken;
	FOnPoiseRecovered OnPoiseRecovered;
	FOnMomentumFilled OnMomentumFilled;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)override;

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)override;

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)override;


	TMap<FGameplayTag, TStaticFuncPtr<FGameplayAttribute()>>TagsToAttributes;


	/*vital Attributes*/
	UPROPERTY(BlueprintReadOnly, Category = "AttributeSets")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UWGasAttributeSet, Health);

	
	UPROPERTY(BlueprintReadOnly, Category = "AttributeSets")
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UWGasAttributeSet, Mana);

	UPROPERTY(BlueprintReadOnly, Category = "AttributeSets")
	FGameplayAttributeData Poise;
	ATTRIBUTE_ACCESSORS(UWGasAttributeSet, Poise);

	UPROPERTY(BlueprintReadOnly, Category = "AttributeSets")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UWGasAttributeSet, Stamina);

	UPROPERTY(BlueprintReadOnly, Category = "AttributeSets")
	FGameplayAttributeData Momentum;
	ATTRIBUTE_ACCESSORS(UWGasAttributeSet, Momentum);

	UPROPERTY(BlueprintReadOnly, Category = "AttributeSets")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UWGasAttributeSet, MaxHealth);

	
	UPROPERTY(BlueprintReadOnly, Category = "AttributeSets")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UWGasAttributeSet, MaxMana);

	UPROPERTY(BlueprintReadOnly, Category = "AttributeSets")
	FGameplayAttributeData MaxPoise;
	ATTRIBUTE_ACCESSORS(UWGasAttributeSet, MaxPoise);

	UPROPERTY(BlueprintReadOnly, Category = "AttributeSets")
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UWGasAttributeSet, MaxStamina);

	UPROPERTY(BlueprintReadOnly, Category = "AttributeSets")
	FGameplayAttributeData MaxMomentum;
	ATTRIBUTE_ACCESSORS(UWGasAttributeSet, MaxMomentum);


	/*Meta Attributes*/
	UPROPERTY(BlueprintReadOnly, Category = "MetaAttributeSets")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UWGasAttributeSet, IncomingDamage);

	
	UPROPERTY(BlueprintReadOnly, Category = "MetaAttributeSets")
	FGameplayAttributeData IncomingPoiseDamage;
	ATTRIBUTE_ACCESSORS(UWGasAttributeSet, IncomingPoiseDamage);



private:

	bool bPoiseBrokenActive = false;
	//从回调数据里解析信息
	void SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props)const;

};
