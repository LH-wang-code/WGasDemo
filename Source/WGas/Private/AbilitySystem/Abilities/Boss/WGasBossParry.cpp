// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Boss/WGasBossParry.h"

#include "AbilitySystemComponent.h"
#include "WGasGameplayTags.h"

UWGasBossParry::UWGasBossParry()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UWGasBossParry::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	bParryEndHandled = false;
	if (UAbilitySystemComponent* ASC = GetWGasASCFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(FWGasGameplayTags::Get().State_Boss_Parry_Success);
	}
	OnBossParryActivated();
}

void UWGasBossParry::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (!bParryEndHandled)
	{
		bParryEndHandled = true;
		CloseParryWindow();

		if (UAbilitySystemComponent* ASC = GetWGasASCFromActorInfo())
		{
			ASC->RemoveLooseGameplayTag(
				FWGasGameplayTags::Get().State_Boss_Parry_Success);
		}
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UWGasBossParry::OpenParryWindow()
{
	if (UAbilitySystemComponent* ASC = GetWGasASCFromActorInfo())
	{
		ASC->AddLooseGameplayTag(FWGasGameplayTags::Get().State_Boss_Parry_Window);
	}
}

void UWGasBossParry::CloseParryWindow()
{
	if (UAbilitySystemComponent* ASC = GetWGasASCFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(FWGasGameplayTags::Get().State_Boss_Parry_Window);
	}
}

bool UWGasBossParry::HasParrySucceeded() const
{
	if (const UAbilitySystemComponent* ASC = GetWGasASCFromActorInfo())
	{
		return ASC->HasMatchingGameplayTag(
			FWGasGameplayTags::Get().State_Boss_Parry_Success);
	}

	return false;
}

void UWGasBossParry::EndBossParry(bool bWasCancelled)
{
	EndAbility(
	CurrentSpecHandle,
	CurrentActorInfo,
	CurrentActivationInfo,
	true,
	bWasCancelled);
}
