// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Character/WGasCharacterBase.h"
#include "WGasCharacterEnemy.generated.h"

class UBossAttackInfo;
/**
 * 
 */
class AWGasAIController;
class UBehaviorTree;
class UMotionWarpingComponent;

UCLASS()
class WGAS_API AWGasCharacterEnemy : public AWGasCharacterBase
{
	GENERATED_BODY()

public:
	AWGasCharacterEnemy();

	virtual void PossessedBy(AController* NewController)override;

	UMotionWarpingComponent* GetMotionWarpingComponent() const { return MotionWarpingComponent; }
	UPROPERTY(EditDefaultsOnly, Category = "AI|Combat")
	TObjectPtr<UBossAttackInfo> AttackSet;
protected:
	ECharacterClass CharacterClass=ECharacterClass::Warrior;
	int32 Level=1;

	//AIController
	UPROPERTY(EditAnywhere,Category="AI")
	TObjectPtr<UBehaviorTree>BehaviorTree;

	UPROPERTY(EditAnywhere, Category = "AI")
	TObjectPtr<AWGasAIController>WGasAIController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Warping")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;


	virtual void BeginPlay() override;

	virtual void InitAbilityActorInfo() override;
	virtual void InitializeDefaultAttributes()const override;

	virtual USkeletalMeshComponent* GetWeaponTraceMesh() const override;
};
