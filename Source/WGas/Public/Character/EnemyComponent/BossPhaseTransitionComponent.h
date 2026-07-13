// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "AbilitySystemComponent.h"
#include "Components/ActorComponent.h"
#include "BossPhaseTransitionComponent.generated.h"

class AWGasCharacterEnemy;
class UBossAIPauseComponent;
class UBossPoiseBrokenComponent;
class UBossAttackInfo;
class UGameplayEffect;
class UGameplayAbility;


UCLASS( ClassGroup=(Boss), meta=(BlueprintSpawnableComponent) )
class WGAS_API UBossPhaseTransitionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	void Initialize(AWGasCharacterEnemy* InOwner,UBossAIPauseComponent* InAIPause,UBossPoiseBrokenComponent* InPoise);

	void BindHealthDelegate();
	void OnPhaseUltimateEnded();
	bool IsTransitionActive() const { return bTransitionTriggered && !bPhase2Entered; }
private:
	TWeakObjectPtr<AWGasCharacterEnemy> OwnerEnemy;
	TWeakObjectPtr<UBossAIPauseComponent> AIPause;
	TWeakObjectPtr<UBossPoiseBrokenComponent> Poise;
	UPROPERTY(EditDefaultsOnly, Category="Phase", meta=(ClampMin="0.01", ClampMax="1.0"))
	float PhaseTransitionHealthPercent = 0.5f;
	UPROPERTY(EditDefaultsOnly, Category="Phase")
	TSubclassOf<UGameplayEffect> PhaseInvincibleEffectClass;
	UPROPERTY(EditDefaultsOnly, Category="Phase")
	TSubclassOf<UGameplayAbility> PhaseTransitionUltimateAbilityClass;
	UPROPERTY(EditDefaultsOnly, Category="Phase")
	TObjectPtr<UBossAttackInfo> Phase2AttackSet;
	UPROPERTY(EditDefaultsOnly, Category="Phase")
	FName PhaseTransitionBlackboardKey = TEXT("bPhaseTransition");
	UPROPERTY(EditDefaultsOnly, Category="Phase")
	FName BossPhaseBlackboardKey = TEXT("BossPhase");
	void OnBossHealthChanged(const FOnAttributeChangeData& Data);
	void BeginPhaseTransition();
	void EnterPhase2();
	void CancelCurrentCombatActions();
	FActiveGameplayEffectHandle PhaseInvincibleEffectHandle;
	bool bTransitionTriggered = false;
	bool bPhase2Entered = false;

		
};
