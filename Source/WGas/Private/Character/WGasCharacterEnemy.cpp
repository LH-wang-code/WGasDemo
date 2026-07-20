// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/WGasCharacterEnemy.h"

#include "AbilitySystem/WGasAbilitySystemComponent.h"
#include "AbilitySystem/WGasAbilitySystemFunctionLibrary.h"
#include "AbilitySystem/WGasAttributeSet.h"
#include "AI/WGasAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Character/EnemyComponent/BossAIPauseComponent.h"
#include "Character/EnemyComponent/BossPhaseTransitionComponent.h"
#include "Character/EnemyComponent/BossPoiseBrokenComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MotionWarpingComponent.h"
#include "WGasGameplayTags.h"
#include "Character/WGasParriedComponent.h"

AWGasCharacterEnemy::AWGasCharacterEnemy()
{
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
	AIPause = CreateDefaultSubobject<UBossAIPauseComponent>(TEXT("AIPause"));
	Poise = CreateDefaultSubobject<UBossPoiseBrokenComponent>(TEXT("Poise"));
	Phase = CreateDefaultSubobject<UBossPhaseTransitionComponent>(TEXT("Phase"));
	ParriedComponent=CreateDefaultSubobject<UWGasParriedComponent>(TEXT("ParriedComponent"));
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->bOrientRotationToMovement = false;
		Movement->bUseControllerDesiredRotation = false;
		Movement->RotationRate = FRotator(0.f, 720.f, 0.f);
	}
}

void AWGasCharacterEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	WGasAIController = Cast<AWGasAIController>(NewController);
	if (WGasAIController && BehaviorTree)
	{
		WGasAIController->RunBehaviorTree(BehaviorTree);
		AddCharacterAbilities();
	}
}

void AWGasCharacterEnemy::BeginPlay()
{
	Super::BeginPlay();
	InitAbilityActorInfo();
}

void AWGasCharacterEnemy::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	InitializeDefaultAttributes();

	if (const UWGasAttributeSet* AS = Cast<UWGasAttributeSet>(AttributeSet))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Init Attr | HP: %.1f/%.1f  Mana: %.1f/%.1f  Poise: %.1f/%.1f  Stamina: %.1f/%.1f"),
			*GetName(),
			AS->GetHealth(), AS->GetMaxHealth(),
			AS->GetMana(), AS->GetMaxMana(),
			AS->GetPoise(), AS->GetMaxPoise(),
			AS->GetStamina(), AS->GetMaxStamina());
	}

	AIPause->Initialize(this, BehaviorTree);
	Poise->Initialize(this, AIPause, Phase);
	Phase->Initialize(this, AIPause, Poise);

	Poise->BindDelegates();
	Phase->BindHealthDelegate();

	if (UWGasAbilitySystemComponent* ASC = Cast<UWGasAbilitySystemComponent>(AbilitySystemComponent))
	{
		ASC->AddLooseGameplayTag(FWGasGameplayTags::Get().State_Boss_Phase_1);
	}
	AIPause->SetBlackboardInt(TEXT("BossPhase"), 1);
}

void AWGasCharacterEnemy::InitializeDefaultAttributes() const
{
	UWGasAbilitySystemFunctionLibrary::InitializeDefaultCharacterClassInfo(this, CharacterClass, Level, AbilitySystemComponent);
}

USkeletalMeshComponent* AWGasCharacterEnemy::GetWeaponTraceMesh() const
{
	if (Weapon && Weapon->GetSkinnedAsset())
	{
		return Weapon;
	}
	return GetMesh();
}

float AWGasCharacterEnemy::GetPoiseBrokenIncomingDamageMultiplier() const
{
	return Poise ? Poise->GetIncomingDamageMultiplier() : 1.f;
}

void AWGasCharacterEnemy::OnPhaseUltimateEnded()
{
	if (Phase)
	{
		Phase->OnPhaseUltimateEnded();
	}
}
