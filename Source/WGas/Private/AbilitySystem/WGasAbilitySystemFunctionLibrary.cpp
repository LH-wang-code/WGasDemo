// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/WGasAbilitySystemFunctionLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Game/WGasGamemode.h"
#include "Kismet/GameplayStatics.h"
#include "WGasEffectTypes.h"
#include "WGasGameplayTags.h"
#include "AbilitySystem/WGasAttributeSet.h"

class UWGasAttributeSet;

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
	const FWGasGameplayTags& WGasTags = FWGasGameplayTags::Get();
	if (WGasTags.State_Invulnerable.IsValid()
		&& Params.TargetAbilitySystemComponent->HasMatchingGameplayTag(WGasTags.State_Invulnerable))
	{
		return;
	}
	FGameplayEffectSpecHandle SpecHandle = Params.SourceAbilitySystemComponent->MakeOutgoingSpec(
		Params.DamageGameplayEffectClass, 1.f, Params.SourceAbilitySystemComponent->MakeEffectContext());

	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, Params.DamageType, Params.BaseDamage);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
		SpecHandle, WGasTags.Data_PoiseDamage, Params.BasePoiseDamage);

	Params.SourceAbilitySystemComponent->ApplyGameplayEffectSpecToTarget(
		*SpecHandle.Data.Get(), Params.TargetAbilitySystemComponent);
}

void UWGasAbilitySystemFunctionLibrary::AddMomentum(UAbilitySystemComponent* ASC, float Amount)
{
	if (!ASC || Amount <= 0.f) return;
	const UWGasAttributeSet* AS = Cast<UWGasAttributeSet>(
	 ASC->GetAttributeSet(UWGasAttributeSet::StaticClass()));
	if (!AS) return;
	const float NewMomentum = FMath::Min(AS->GetMomentum() + Amount, AS->GetMaxMomentum());
	ASC->SetNumericAttributeBase(UWGasAttributeSet::GetMomentumAttribute(), NewMomentum);
}
