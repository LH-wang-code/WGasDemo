// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WGasAbilitySystemFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class WGAS_API UWGasAbilitySystemFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


public:

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary| CharacterClassInfo")
	static UCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary| CharacterClassInfo")
	static void InitializeDefaultCharacterClassInfo(const UObject* WorldContextObject,ECharacterClass CharacterClass, float level,UAbilitySystemComponent* ASC);

};
