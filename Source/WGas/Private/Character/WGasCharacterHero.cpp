// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/WGasCharacterHero.h"

#include "AbilitySystem/WGasAbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
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

	CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;


	InputBufferComponent = CreateDefaultSubobject<UGasInputBufferComponent>("InputBufferComponent");
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = false;
	Movement->bUseControllerDesiredRotation = false;

	ApplyMovementSpeed();
}

void AWGasCharacterHero::ToggleWalkRun()
{
	bIsRunning = !bIsRunning;
	ApplyMovementSpeed();
}

void AWGasCharacterHero::ApplyMovementSpeed()
{
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = bIsRunning ? RunSpeed : WalkSpeed;
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
}
