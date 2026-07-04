// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/WGasCharacterHero.h"

#include "AbilitySystem/WGasAbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

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
	// 角色 Yaw 跟随相机，S 键才能后退平移而不是转身
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = false;
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
}
