// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ExecCalc_Damage.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "WGasGameplayTags.h"
#include "AbilitySystem/WGasAttributeSet.h"
#include "AbilitySystem/Abilities/WGasBlock.h"
#include "AbilitySystem/Abilities/WGasMeleeAttack.h"
#include "AbilitySystem/Abilities/Boss/WGasBossMeleeAttack.h"
#include "Character/WGasCharacterBase.h"
#include "Character/WGasCharacterHero.h"
#include "Character/WGasParriedComponent.h"
#include "Interaction/BossCombatInterface.h"

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
			if (const IBossCombatInterface* BossCombat = Cast<IBossCombatInterface>(TargetActor))
			{
				Damage *= BossCombat->GetBossIncomingDamageMultiplier();
			}
		}
	}
	
	//格挡与免伤
	if (UAbilitySystemComponent* TargetASC=ExecutionParams.GetTargetAbilitySystemComponent())
	{
		if (WGasTags.State_Boss_Parry_Window.IsValid() &&
		TargetASC->HasMatchingGameplayTag(WGasTags.State_Boss_Parry_Window))
		{
			UAbilitySystemComponent* SourceASC =ExecutionParams.GetSourceAbilitySystemComponent();
			if (SourceASC)
			{
				if (AWGasCharacterHero* Hero =
					Cast<AWGasCharacterHero>(SourceASC->GetAvatarActor()))
				{
					// Boss 记录本次弹反成功，供 GA_BossParry 蓝图判断。
					if (WGasTags.State_Boss_Parry_Success.IsValid())
					{
						TargetASC->AddLooseGameplayTag(
							WGasTags.State_Boss_Parry_Success);
					}

					// 中断玩家正在进行的近战技能。
					if (UWGasMeleeAttack* Melee =
						UWGasMeleeAttack::GetActiveMeleeAttack(SourceASC))
					{
						Melee->EndMeleeAttack(true);
					}

					// 玩家播放受击硬直；传入 1 仅用于触发硬直，不额外扣血。
					Hero->NotifyHitReact(1.f);

					return; // 不向 Boss 结算此次伤害
				}
			}
		}
		if (WGasTags.State_Boss_Greatsword_Block.IsValid()&& TargetASC->HasMatchingGameplayTag(WGasTags.State_Boss_Greatsword_Block))
		{
			if (IBossCombatInterface* BossCombat = Cast<IBossCombatInterface>(TargetASC->GetAvatarActor()))
			{
				if (BossCombat->IsBossPhase2())
				{
					BossCombat->NotifyBossGuardedHit();
				}
			}
			Damage *= 0.35f;
		}
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
			UAbilitySystemComponent* AttackerASC = ExecutionParams.GetSourceAbilitySystemComponent();
			if (AttackerASC)
			{
				if (AWGasCharacterBase* Attacker = Cast<AWGasCharacterBase>(AttackerASC->GetAvatarActor()))
				{
					if (UWGasParriedComponent* Parried = Attacker->FindComponentByClass<UWGasParriedComponent>())
					{
						Parried->EnterParried(TargetASC->GetAvatarActor());
					}
				}
			}
			UWGasBossMeleeAttack::TryInterruptFromParry(AttackerASC);
			return; // 完全免伤
		}
	}
	
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UWGasAttributeSet::GetIncomingDamageAttribute(),EGameplayModOp::Additive,Damage));
}
