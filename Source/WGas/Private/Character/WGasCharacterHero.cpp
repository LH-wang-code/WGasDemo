// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/WGasCharacterHero.h"

#include "MotionWarpingComponent.h"
#include "WGasGameplayTags.h"
#include "AbilitySystem/WGasAbilitySystemComponent.h"
#include "AbilitySystem/WGasAttributeSet.h"
#include "Camera/CameraComponent.h"
#include "Character/WGasLockOnComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Player/WGasPlayerController.h"
#include "UI/HUD/WGasHUD.h"

AWGasCharacterHero::AWGasCharacterHero()
{
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArmComponent");
	SpringArmComponent->SetupAttachment(GetRootComponent());
	SpringArmComponent->TargetArmLength = 400.f;
	SpringArmComponent->bUsePawnControlRotation = true;
	SpringArmComponent->bInheritPitch = true;
	SpringArmComponent->bInheritYaw = true;
	SpringArmComponent->bInheritRoll = false;
	SpringArmComponent->SetUsingAbsoluteRotation(false);
	SpringArmComponent->bDoCollisionTest = false;
	SpringArmComponent->TargetOffset = FVector(0.f, 0.f, 100.f); 
	CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;


	LockOnComponent=CreateDefaultSubobject<UWGasLockOnComponent>("LockOnComponent");

	InputBufferComponent = CreateDefaultSubobject<UGasInputBufferComponent>("InputBufferComponent");
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = false;
	Movement->bUseControllerDesiredRotation = false;

	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
	
	ApplyMovementSpeed();
}

void AWGasCharacterHero::ToggleWalkRun()
{
	if (!bIsRunning)
	{
		if (const UWGasAttributeSet* AS = Cast<UWGasAttributeSet>(AttributeSet))
		{
			if (AS->GetStamina() <= 0.f)
			{
				return; // 体力不够，不切跑
			}
		}
	}
	bIsRunning = !bIsRunning;
	ApplyMovementSpeed();
	if (!bIsRunning)
	{
		ClearRunningTag();
	}
}

void AWGasCharacterHero::ForceWalk()
{
	if (!bIsRunning)
	{
		return; 
	}
	bIsRunning = false;
	ApplyMovementSpeed();
	ClearRunningTag();
}

void AWGasCharacterHero::UpdateRunningTag(const FVector2D& MoveInput)
{
	UWGasAbilitySystemComponent* ASC = Cast<UWGasAbilitySystemComponent>(AbilitySystemComponent);
	if (!ASC)
	{
		return;
	}

	const UWGasAttributeSet* AS = Cast<UWGasAttributeSet>(AttributeSet);
	if (bIsRunning && AS && AS->GetStamina() <= 0.f)
	{
		ForceWalk();
		return;
	}

	const bool bShouldConsumeStamina = bIsRunning
		&& !MoveInput.IsNearlyZero()
		&& AS
		&& AS->GetStamina() > 0.f;
	const FGameplayTag& RunningTag = FWGasGameplayTags::Get().State_Running;
	const bool bHasRunningTag = ASC->HasMatchingGameplayTag(RunningTag);

	if (bShouldConsumeStamina)
	{
		if (!bHasRunningTag)
		{
			ASC->AddLooseGameplayTag(RunningTag);
		}
	}
	else if (bHasRunningTag)
	{
		ClearRunningTag();
	}
}

void AWGasCharacterHero::ClearRunningTag()
{
	if (UWGasAbilitySystemComponent* ASC = Cast<UWGasAbilitySystemComponent>(AbilitySystemComponent))
	{
		const FGameplayTag& RunningTag = FWGasGameplayTags::Get().State_Running;
		ASC->SetLooseGameplayTagCount(RunningTag, 0);
	}
}

void AWGasCharacterHero::ApplyMovementSpeed()
{
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = bIsRunning ? RunSpeed : WalkSpeed;
	}
}

void AWGasCharacterHero::BindStaminaDepletedDelegate()
{
	if (bStaminaDepletedBound) return;
	if (UWGasAttributeSet* AS = Cast<UWGasAttributeSet>(AttributeSet))
	{
		AS->OnStaminaDepleted.AddUObject(this, &AWGasCharacterHero::ForceWalk);
		bStaminaDepletedBound = true;
	}
}
void AWGasCharacterHero::PossessedBy(AController* NewController)
{
	//角色被控制时添加能力
	Super::PossessedBy(NewController);
	InitAbilityActorInfo();
	AddCharacterAbilities();
}

void AWGasCharacterHero::InitAbilityActorInfo()
{
	Super::InitAbilityActorInfo();

	AWGasPlayerController* PC=Cast<AWGasPlayerController>(GetController());
	if (!PC)return;
	auto InitHUD=[this,PC]()
	{
		if (AWGasHUD* WGasHUD=Cast<AWGasHUD>(PC->GetHUD()))
		{
			WGasHUD->InitOverlay(PC,AbilitySystemComponent,AttributeSet);
			
		}
	};

	if (AWGasHUD* WGasHUD=Cast<AWGasHUD>(PC->GetHUD()))
	{
		InitHUD();
	}
	else
	{
		FTimerHandle Handle;
		GetWorldTimerManager().SetTimer(
			Handle,
			InitHUD,
			0.0f,   
			false
		);
	}
	InitializeDefaultAttributes();
	BindStaminaDepletedDelegate();

	//这里处理的不好，先暂时这样放着
	if (!bRunStaminaEffectsApplied && StaminaCostGE)
	{
		ApplyEffectToSelf(StaminaCostGE, 1.f);
		bRunStaminaEffectsApplied = true;
	}
	
}
