// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ExecCalc_Damage.h"

#include "AbilitySystemComponent.h"
#include "WGasGameplayTags.h"
#include "AbilitySystem/WGasAttributeSet.h"
#include "AbilitySystem/Abilities/WGasBlock.h"
#include "Character/WGasCharacterBase.h"
#include "Character/WGasCharacterEnemy.h"
#include "Character/WGasCharacterHero.h"

UExecCalc_Damage::UExecCalc_Damage()
{
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec=ExecutionParams.GetOwningSpec();
	const FGameplayTagContainer* TargetTags=Spec.CapturedTargetTags.GetAggregatedTags();
	const FGameplayTag& PoiseBrokenTag=FWGasGameplayTags::Get().State_Boss_PoiseBroken;

	const FWGasGameplayTags& WGasTags = FWGasGameplayTags::Get();
	if (UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent())
	{
		if (WGasTags.State_Boss_Invulnerable.IsValid()
			&& TargetASC->HasMatchingGameplayTag(WGasTags.State_Boss_Invulnerable))
		{
			return;
		}
		if (WGasTags.State_Invulnerable.IsValid()
			&& TargetASC->HasMatchingGameplayTag(WGasTags.State_Invulnerable))
		{
			return;
		}
	}

	float Damage=0.f;
	for (const TPair<FGameplayTag,float>&Pair:Spec.SetByCallerTagMagnitudes)
	{
		if (Pair.Key == FWGasGameplayTags::Get().Data_PoiseDamage)
		{
			continue;
		}
		Damage += Pair.Value;
	}
	//boss破韧
	if (TargetTags&&PoiseBrokenTag.IsValid() &&TargetTags->HasTag(PoiseBrokenTag))
	{
		if (const AActor* TargetActor=ExecutionParams.GetTargetAbilitySystemComponent()->GetAvatarActor())
		{
			if (const AWGasCharacterEnemy* Enemy=Cast<AWGasCharacterEnemy>(TargetActor))
			{
				Damage *= Enemy->GetPoiseBrokenIncomingDamageMultiplier();
			}
		}
	}

	//格挡与免伤
	if (UAbilitySystemComponent* TargetASC=ExecutionParams.GetTargetAbilitySystemComponent())
	{
		if (WGasTags.State_Block.IsValid()&& TargetASC->HasMatchingGameplayTag(WGasTags.State_Block))
		{
			Damage *= UWGasBlock::GetBlockDamageMultiplierForTarget(TargetASC);
		}
		if (WGasTags.State_Parry_Window.IsValid()&& TargetASC->HasMatchingGameplayTag(WGasTags.State_Parry_Window))
		{
			TargetASC->AddLooseGameplayTag(WGasTags.State_Parry_Success);
			if (AWGasCharacterHero* Hero = Cast<AWGasCharacterHero>(TargetASC->GetAvatarActor()))
			{
				Hero->NotifyParrySuccess();
			}
			return; // 完全免伤
		}
	}
	
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UWGasAttributeSet::GetIncomingDamageAttribute(),EGameplayModOp::Additive,Damage));
}
