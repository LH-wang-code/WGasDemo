// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/WGasPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Input/WGasInputComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Engine/GameViewportClient.h"
#include "TimerManager.h"
#include "AbilitySystem/WGasAbilitySystemComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "WGasGameplayTags.h"

struct FGameplayAbilitySpec;

void AWGasPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (UWGasAbilitySystemComponent* ASC = GetASC())
	{
		if (ASC->HasMatchingGameplayTag(FWGasGameplayTags::Get().Player_Block_InputPressed))
		{
			return;
		}

		ASC->AbilityInputTagPressed(InputTag);
	}
}

void AWGasPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (UWGasAbilitySystemComponent* ASC = GetASC())
	{
		if (ASC->HasMatchingGameplayTag(FWGasGameplayTags::Get().Player_Block_InputHeld))
		{
			return;
		}

		ASC->AbilityInputTagHeld(InputTag);
	}
}

void AWGasPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (UWGasAbilitySystemComponent* ASC = GetASC())
	{
		if (ASC->HasMatchingGameplayTag(FWGasGameplayTags::Get().Player_Block_InputReleased))
		{
			return;
		}

		ASC->AbilityInputTagReleased(InputTag);
	}
}

void AWGasPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalPlayerController())
	{
		return;
	}

	if (WGasContext)
	{
		if (UEnhancedInputLocalPlayerSubsystem* SubSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			SubSystem->AddMappingContext(WGasContext, 0);
		}
	}

	GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		bShowMouseCursor = false;
		bEnableClickEvents = false;
		bEnableMouseOverEvents = false;

		FInputModeGameOnly InputMode;
		InputMode.SetConsumeCaptureMouseDown(true);
		SetInputMode(InputMode);

		if (UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport())
		{
			ViewportClient->SetMouseCaptureMode(EMouseCaptureMode::CapturePermanently);
			ViewportClient->SetMouseLockMode(EMouseLockMode::LockOnCapture);
		}
	}));
}

void AWGasPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	//我们自己创建了一个enhanceinput，强转之前需要在defaultinput.ini重新设置
	UWGasInputComponent* WGasInputComponent = Cast<UWGasInputComponent>(InputComponent);
	if (!WGasInputComponent)
	{
		return;
	}

	if (MoveAction)
	{
		WGasInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AWGasPlayerController::Move);
	}

	if (LookAction)
	{
		WGasInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AWGasPlayerController::Look);
	}

	if (JumpAction)
	{
		WGasInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AWGasPlayerController::Jump);
		WGasInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AWGasPlayerController::StopJumping);
	}
	if (InputConfig)
	{
		WGasInputComponent->BindAbilityActions(
			InputConfig,
			this,
			&ThisClass::AbilityInputTagPressed,
			&ThisClass::AbilityInputTagReleased,
			&ThisClass::AbilityInputTagHeld);
	}
}

void AWGasPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	WGasASC = nullptr;
}
//控制前后左右
void AWGasPlayerController::Move(const FInputActionValue& InputActionValue)
{
	if (UWGasAbilitySystemComponent* ASC = GetASC())
	{
		const FWGasGameplayTags& GameplayTags = FWGasGameplayTags::Get();
		if (ASC->HasMatchingGameplayTag(GameplayTags.State_Attacking_Lighting)
			|| ASC->HasMatchingGameplayTag(GameplayTags.State_Dodge))
		{
			return;
		}
	}
	const FVector2D InputAxisVector=InputActionValue.Get<FVector2D>();
	const FRotator Rotation=GetControlRotation();
	const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}

void AWGasPlayerController::Look(const FInputActionValue& InputActionValue)
{
	const FVector2D LookAxis = InputActionValue.Get<FVector2D>();
	if (LookAxis.IsNearlyZero())
	{
		return;
	}

	AddYawInput(LookAxis.X);
	AddPitchInput(LookAxis.Y);
}

void AWGasPlayerController::Jump(const FInputActionValue& InputActionValue)
{
	if (UWGasAbilitySystemComponent* ASC = GetASC())
	{
		const FWGasGameplayTags& GameplayTags = FWGasGameplayTags::Get();
		if (ASC->HasMatchingGameplayTag(GameplayTags.State_Attacking_Lighting)
			|| ASC->HasMatchingGameplayTag(GameplayTags.State_Dodge))
		{
			return;
		}
	}
	if (ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn()))
	{
		ControlledCharacter->Jump();
	}
}

void AWGasPlayerController::StopJumping(const FInputActionValue& InputActionValue)
{
	if (ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn()))
	{
		ControlledCharacter->StopJumping();
	}
}
//playercontroller存一个
UWGasAbilitySystemComponent* AWGasPlayerController::GetASC()
{
	if (WGasASC == nullptr)
	{
		WGasASC=Cast<UWGasAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn()));
	}
	return WGasASC;
}
