// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/EnemyComponent/BossPhaseTransitionComponent.h"

#include "Abilities/GameplayAbility.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "WGasGameplayTags.h"
#include "AbilitySystem/WGasAbilitySystemComponent.h"
#include "AbilitySystem/WGasAttributeSet.h"
#include "Character/WGasCharacterEnemy.h"
#include "Character/EnemyComponent/BossPoiseBrokenComponent.h"
#include "Character/EnemyComponent/BossAIPauseComponent.h"

class UWGasAttributeSet;

void UBossPhaseTransitionComponent::Initialize(AWGasCharacterEnemy* InOwner, UBossAIPauseComponent* InAIPause,
                                               UBossPoiseBrokenComponent* InPoise)
{
	OwnerEnemy = InOwner;
	AIPause = InAIPause;
	Poise = InPoise;
}

void UBossPhaseTransitionComponent::BindHealthDelegate()
{
	AWGasCharacterEnemy* Enemy = OwnerEnemy.Get();
	if (!Enemy || !Enemy->GetAbilitySystemComponent()) return;
	
	if (UWGasAttributeSet* AS = Cast<UWGasAttributeSet>(Enemy->GetAttributeSet()))
	{
		Enemy->GetAbilitySystemComponent()->GetGameplayAttributeValueChangeDelegate(
			AS->GetHealthAttribute()).AddUObject(this, &UBossPhaseTransitionComponent::OnBossHealthChanged);
	}
}

void UBossPhaseTransitionComponent::OnPhaseUltimateEnded()
{
	EnterPhase2();
}

void UBossPhaseTransitionComponent::OnBossHealthChanged(const FOnAttributeChangeData& Data)
{
	if (bTransitionTriggered)return;
	AWGasCharacterEnemy* Enemy=OwnerEnemy.Get();
	if (!Enemy)return;
	UWGasAttributeSet* AS=Cast<UWGasAttributeSet>(Enemy->GetAttributeSet());
	if (!AS)return;
	if (AS->GetHealth()/AS->GetMaxHealth() > PhaseTransitionHealthPercent)return;
	const float FloorHealth = AS->GetMaxHealth() * PhaseTransitionHealthPercent;
	if (AS->GetHealth() < FloorHealth)
	{
		AS->SetHealth(FloorHealth);
	}
	bTransitionTriggered = true;
	BeginPhaseTransition();
}

void UBossPhaseTransitionComponent::BeginPhaseTransition()
{
	AWGasCharacterEnemy* Enemy = OwnerEnemy.Get();
	UBossAIPauseComponent* AIPauseComp = AIPause.Get();
	if (!Enemy || !AIPauseComp)return;
	UWGasAbilitySystemComponent* ASC = Cast<UWGasAbilitySystemComponent>(Enemy->GetAbilitySystemComponent());
	if (!ASC)return;
	const FWGasGameplayTags& WGasTags = FWGasGameplayTags::Get();
	CancelCurrentCombatActions();
	AIPauseComp->StopMovement();
	AIPauseComp->PauseBrain(TEXT("PhaseTransition"));

	AIPauseComp->SetBlackboardBool(PhaseTransitionBlackboardKey, true);
	if (WGasTags.State_Boss_PhaseTransition.IsValid())
	{
		ASC->AddLooseGameplayTag(WGasTags.State_Boss_PhaseTransition);
	}
	if (WGasTags.State_Boss_Phase_1.IsValid())
	{
		ASC->RemoveLooseGameplayTag(WGasTags.State_Boss_Phase_1);
	}
	if (PhaseInvincibleEffectClass)
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		Context.AddSourceObject(Enemy);
		const FGameplayEffectSpecHandle SpecHandle =
			ASC->MakeOutgoingSpec(PhaseInvincibleEffectClass, 1.f, Context);
		PhaseInvincibleEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
	if (PhaseTransitionUltimateAbilityClass)
	{
		FGameplayAbilitySpec Spec(PhaseTransitionUltimateAbilityClass, 1, INDEX_NONE, Enemy);
		const FGameplayAbilitySpecHandle ActivatedHandle = ASC->GiveAbilityAndActivateOnce(Spec);
		if (!ActivatedHandle.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] Phase ultimate failed to activate."), *Enemy->GetName());
			OnPhaseUltimateEnded();
		}
		return;
	}
	OnPhaseUltimateEnded();
}

void UBossPhaseTransitionComponent::EnterPhase2()
{
	if (bPhase2Entered)return;
	bPhase2Entered = true;
	AWGasCharacterEnemy* Enemy = OwnerEnemy.Get();
	UBossAIPauseComponent* AIPauseComp = AIPause.Get();
	if (!Enemy || !AIPauseComp)return;
	UWGasAbilitySystemComponent* ASC = Cast<UWGasAbilitySystemComponent>(Enemy->GetAbilitySystemComponent());
	const FWGasGameplayTags& WGasTags = FWGasGameplayTags::Get();
	if (ASC)
	{
		if (PhaseInvincibleEffectHandle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(PhaseInvincibleEffectHandle);
			PhaseInvincibleEffectHandle.Invalidate();
		}
		if (WGasTags.State_Boss_Invulnerable.IsValid())
		{
			ASC->RemoveLooseGameplayTag(WGasTags.State_Boss_Invulnerable);
		}
		if (WGasTags.State_Boss_PhaseTransition.IsValid())
		{
			ASC->RemoveLooseGameplayTag(WGasTags.State_Boss_PhaseTransition);
		}
		if (WGasTags.State_Boss_Phase_2.IsValid())
		{
			ASC->AddLooseGameplayTag(WGasTags.State_Boss_Phase_2);
		}
	}
	if (Phase2AttackSet)
	{
		Enemy->AttackSet = Phase2AttackSet;
	}
	AIPauseComp->SetBlackboardBool(PhaseTransitionBlackboardKey, false);
	AIPauseComp->SetBlackboardInt(BossPhaseBlackboardKey, 2);
	AIPauseComp->ResetDefaultMovement();
	AIPauseComp->CleanupPathAndFocus();
	AIPauseComp->ResumeBrain(TEXT("PhaseTransition"));
}

void UBossPhaseTransitionComponent::CancelCurrentCombatActions()
{
	AWGasCharacterEnemy* Enemy = OwnerEnemy.Get();
	UBossAIPauseComponent* AIPauseComp = AIPause.Get();
	UBossPoiseBrokenComponent* PoiseComp = Poise.Get();
	if (!Enemy)return;
	UWGasAbilitySystemComponent*ASC=Cast<UWGasAbilitySystemComponent>(Enemy->GetAbilitySystemComponent());
	if (!ASC)return;
	const FWGasGameplayTags& WGasTags=FWGasGameplayTags::Get();
	if (WGasTags.State_Boss_Attacking.IsValid())
	{
		FGameplayTagContainer CancelTags;
		CancelTags.AddTag(WGasTags.State_Boss_Attacking);
		ASC->CancelAbilities(&CancelTags);
	}
	if (Enemy->CombatComponent)
	{
		Enemy->CombatComponent->EndWeaponSweep();
	}
	if (PoiseComp&& PoiseComp->IsPoiseBrokenActive())
	{
		if (USkeletalMeshComponent* MeshComp = Enemy->GetMesh())
		{
			if (UAnimInstance* Anim = MeshComp->GetAnimInstance())
			{
				// Montage 在 Poise 组件里，这里 Stop 全部或让 Poise 提供 StopMontage()
				Anim->Montage_Stop(0.1f);
			}
		}
		PoiseComp->ForceCleanupWithoutResumeBrain();
	}
	if (AIPauseComp)
	{
		AIPauseComp->ClearPauseReason(TEXT("PoiseBroken"));
	}
}


