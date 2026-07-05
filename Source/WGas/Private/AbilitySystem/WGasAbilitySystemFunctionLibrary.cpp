// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/WGasAbilitySystemFunctionLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Game/WGasGamemode.h"
#include "Kismet/GameplayStatics.h"
#include "WGasEffectTypes.h"

UCharacterClassInfo* UWGasAbilitySystemFunctionLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	AWGasGamemode* GM=Cast<AWGasGamemode>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (!GM)return nullptr;
	return GM->CharacterClassInfo;
}

void UWGasAbilitySystemFunctionLibrary::InitializeDefaultCharacterClassInfo(const UObject* WorldContextObject,
                                                                            ECharacterClass CharacterClass, float level, UAbilitySystemComponent* ASC)
{
	AActor* AvatarActor=ASC->GetAvatarActor();
	UCharacterClassInfo* CharacterClassInfo=GetCharacterClassInfo(WorldContextObject);
	if (!CharacterClassInfo) return ;

	FCharacterClassDefaultInfo CharacterClassDefaultInfo = CharacterClassInfo->GetInfoWithECharacterClass(CharacterClass);

	FGameplayEffectContextHandle SecondaryEffectContextHandle = ASC->MakeEffectContext();
	SecondaryEffectContextHandle.AddSourceObject(AvatarActor);
	FGameplayEffectSpecHandle SecondaryEffectSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->SecondaryAttributes, level, SecondaryEffectContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*SecondaryEffectSpecHandle.Data.Get());

	
	FGameplayEffectContextHandle VitalEffectContextHandle = ASC->MakeEffectContext();
	VitalEffectContextHandle.AddSourceObject(AvatarActor);
	FGameplayEffectSpecHandle VitalEffectSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->VitalAttributes, level, VitalEffectContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*VitalEffectSpecHandle.Data.Get());
}

void UWGasAbilitySystemFunctionLibrary::ApplyDamageEffectParams(const FDamageEffectParams& Params)
{
	if (!Params.DamageGameplayEffectClass|| !Params.SourceAbilitySystemComponent|| !Params.TargetAbilitySystemComponent)
	{
		return;
	}

	FGameplayEffectSpecHandle SpecHandle =Params.SourceAbilitySystemComponent->MakeOutgoingSpec(Params.DamageGameplayEffectClass, 1.f,Params.SourceAbilitySystemComponent->MakeEffectContext());

	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
		SpecHandle, Params.DamageType, Params.BaseDamage);
	Params.SourceAbilitySystemComponent->ApplyGameplayEffectSpecToTarget(
		*SpecHandle.Data.Get(), Params.TargetAbilitySystemComponent);
}
