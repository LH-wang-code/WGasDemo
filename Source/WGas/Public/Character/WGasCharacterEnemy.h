// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Character/WGasCharacterBase.h"
#include "WGasCharacterEnemy.generated.h"

class UBossAttackInfo;
class AWGasAIController;
class UBehaviorTree;
class UMotionWarpingComponent;
class UBossAIPauseComponent;
class UBossPoiseBrokenComponent;
class UBossPhaseTransitionComponent;
class UWGasParriedComponent;
UCLASS()
class WGAS_API AWGasCharacterEnemy : public AWGasCharacterBase
{
	GENERATED_BODY()

public:
	AWGasCharacterEnemy();

	virtual void PossessedBy(AController* NewController) override;

	UMotionWarpingComponent* GetMotionWarpingComponent() const { return MotionWarpingComponent; }
	UBossAIPauseComponent* GetAIPauseComponent() const { return AIPause; }
	UBossPoiseBrokenComponent* GetPoiseComponent() const { return Poise; }
	UBossPhaseTransitionComponent* GetPhaseComponent() const { return Phase; }

	UPROPERTY(EditDefaultsOnly, Category = "AI|Combat")
	TObjectPtr<UBossAttackInfo> AttackSet;

	float GetPoiseBrokenIncomingDamageMultiplier() const;

	void OnPhaseUltimateEnded();

protected:
	ECharacterClass CharacterClass = ECharacterClass::Warrior;
	int32 Level = 1;

	UPROPERTY(EditAnywhere, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(EditAnywhere, Category = "AI")
	TObjectPtr<AWGasAIController> WGasAIController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Warping")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss")
	TObjectPtr<UBossAIPauseComponent> AIPause;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss")
	TObjectPtr<UBossPoiseBrokenComponent> Poise;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss")
	TObjectPtr<UBossPhaseTransitionComponent> Phase;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss")
	TObjectPtr<UWGasParriedComponent> ParriedComponent;
	virtual void BeginPlay() override;

	virtual void InitAbilityActorInfo() override;
	virtual void InitializeDefaultAttributes() const override;

	virtual USkeletalMeshComponent* GetWeaponTraceMesh() const override;

	virtual void HandleDeathExtras() override;
};
