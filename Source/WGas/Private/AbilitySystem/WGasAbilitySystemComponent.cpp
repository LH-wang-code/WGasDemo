// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/WGasAbilitySystemComponent.h"

#include "AbilitySystem/Abilities/WGasGameplayAbility.h"
#include "WGasGameplayTags.h"

void UWGasAbilitySystemComponent::AbilityActorInfoSet()
{
}

void UWGasAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (const TSubclassOf<UGameplayAbility>AbilityClass:StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec=FGameplayAbilitySpec(AbilityClass,1);
		if (const UWGasGameplayAbility* WGasAbility=Cast<UWGasGameplayAbility>(AbilitySpec.Ability))
		{
			//初始能力以及按键的绑定
			AbilitySpec.DynamicAbilityTags.AddTag(WGasAbility->StartupInputTag);
			GiveAbility(AbilitySpec);
		}
	}
}

void UWGasAbilitySystemComponent::AddCharacterPassiveAbilities(
	const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities)
{
	for (const TSubclassOf<UGameplayAbility>& AbilityClass : StartupPassiveAbilities)
	{
		if (!AbilityClass) continue;
		FGameplayAbilitySpec AbilitySpec(AbilityClass, 1);
		GiveAbility(AbilitySpec);   
	}
}

void UWGasAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);

			if (AbilitySpec.IsActive())
			{
				// 直接使用 AbilitySpec 的 ActivationInfo
				InvokeReplicatedEvent(
					EAbilityGenericReplicatedEvent::InputPressed,
					AbilitySpec.Handle,
					AbilitySpec.ActivationInfo.GetActivationPredictionKey()
				);
			}
		}
	}
}

void UWGasAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	const FWGasGameplayTags& Tags = FWGasGameplayTags::Get();
	if (Tags.State_Dodge.IsValid() && HasMatchingGameplayTag(Tags.State_Dodge)
		&& InputTag.MatchesTagExact(Tags.InputTag_Ctrl))
	{
		return;
	}

	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			if (!AbilitySpec.IsActive())
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

void UWGasAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag) && AbilitySpec.IsActive())
		{
			AbilitySpecInputReleased(AbilitySpec);

			//
			TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
			const FGameplayAbilityActivationInfo& ActivationInfo = Instances.IsEmpty() ? AbilitySpec.ActivationInfo : Instances.Last()->GetCurrentActivationInfoRef();
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilitySpec.Handle, ActivationInfo.GetActivationPredictionKey());
		}
	}
}
