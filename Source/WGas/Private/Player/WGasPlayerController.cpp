#include "Player/WGasPlayerController.h"

#include "Character/WGasCharacterHero.h"
#include "Character/WGasCharacterBase.h"
#include "EnhancedInputSubsystems.h"
#include "Input/WGasInputComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Engine/GameViewportClient.h"
#include "TimerManager.h"
#include "AbilitySystem/WGasAbilitySystemComponent.h"
#include "AbilitySystem/WGasAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "WGasGameplayTags.h"
#include "AbilitySystem/Abilities/WGasBlock.h"
#include "AbilitySystem/Abilities/WGasMeleeAttack.h"
#include "Character/WGasLockOnComponent.h"
#include "UI/Widgets/StaminaBarComponent.h"
#include "UI/Widgets/WGasStaminaBarWidget.h"
#include "UI/WidgetController/StaminaBarWGasWidgetController.h"

struct FGameplayAbilitySpec;

void AWGasPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (UWGasAbilitySystemComponent* ASC = GetASC())
	{
		const FWGasGameplayTags& WGasTags = FWGasGameplayTags::Get();
		if (WGasTags.InputTag_LMB.IsValid()
			&& InputTag.MatchesTagExact(WGasTags.InputTag_LMB)
			&& WGasTags.State_Block.IsValid()
			&& ASC->HasMatchingGameplayTag(WGasTags.State_Block))
		{
			if (UWGasBlock* Block = UWGasBlock::GetActiveBlock(ASC))
			{
				Block->TryParryFromAttackInput();
				return;
			}
		}

		if (ASC->HasMatchingGameplayTag(WGasTags.Player_Block_InputPressed))
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
		const FWGasGameplayTags& WGasTags = FWGasGameplayTags::Get();
		if (WGasTags.InputTag_LMB.IsValid()
			&& InputTag.MatchesTagExact(WGasTags.InputTag_LMB)
			&& WGasTags.State_Block.IsValid()
			&& ASC->HasMatchingGameplayTag(WGasTags.State_Block))
		{
			return;
		}

		if (ASC->HasMatchingGameplayTag(WGasTags.Player_Block_InputHeld))
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

	UWGasInputComponent* WGasInputComponent = Cast<UWGasInputComponent>(InputComponent);
	if (!WGasInputComponent)
	{
		return;
	}

	if (MoveAction)
	{
		WGasInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AWGasPlayerController::Move);
		WGasInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AWGasPlayerController::StopMove);
		WGasInputComponent->BindAction(MoveAction, ETriggerEvent::Canceled, this, &AWGasPlayerController::StopMove);
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

	if (ToggleWalkRunAction)
	{
		WGasInputComponent->BindAction(ToggleWalkRunAction, ETriggerEvent::Started, this, &AWGasPlayerController::ToggleWalkRun);
	}

	if (ToggleLockOnAction)
	{
		WGasInputComponent->BindAction(ToggleLockOnAction, ETriggerEvent::Started, this, &AWGasPlayerController::ToggleLockOnInput);
		
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

	if (IsLocalPlayerController())
	{
		SetupStaminaBar(InPawn);
	}
}

void AWGasPlayerController::OnUnPossess()
{
	TeardownStaminaBar();
	Super::OnUnPossess();
}

void AWGasPlayerController::SetupStaminaBar(APawn* InPawn)
{
	TeardownStaminaBar();

	if (!InPawn || !StaminaBarComponentClass || !StaminaBarWidgetClass || !StaminaWidgetControllerClass)
	{
		return;
	}

	StaminaBarComponentInstance = NewObject<UStaminaBarComponent>(InPawn, StaminaBarComponentClass);
	StaminaBarComponentInstance->SetWidgetClass(StaminaBarWidgetClass);
	StaminaBarComponentInstance->SetupAttachment(InPawn->GetRootComponent());
	StaminaBarComponentInstance->SetRelativeLocation(StaminaBarRelativeLocation);
	StaminaBarComponentInstance->RegisterComponent();
	StaminaBarComponentInstance->InitWidget();

	GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		BindStaminaBar();
	}));
}

void AWGasPlayerController::BindStaminaBar()
{
	UWGasAbilitySystemComponent* ASC = GetASC();
	if (!ASC || !StaminaBarComponentInstance || !StaminaWidgetControllerClass)
	{
		return;
	}

	UWGasAttributeSet* AttributeSet = nullptr;
	if (const AWGasCharacterBase* GasCharacter = Cast<AWGasCharacterBase>(GetPawn()))
	{
		AttributeSet = Cast<UWGasAttributeSet>(GasCharacter->GetAttributeSet());
	}
	if (!AttributeSet)
	{
		return;
	}

	StaminaWidgetController = NewObject<UStaminaBarWGasWidgetController>(this, StaminaWidgetControllerClass);
	const FWidgetControllerParams WidgetParams(this, ASC, AttributeSet);
	StaminaWidgetController->SetWidgetControllerParams(WidgetParams);
	StaminaWidgetController->BindCallbacksToDependencies();

	if (UWGasStaminaBarWidget* StaminaWidget = Cast<UWGasStaminaBarWidget>(StaminaBarComponentInstance->GetWidget()))
	{
		StaminaWidget->SetWidgetController(StaminaWidgetController);
		StaminaWidgetController->BroadcastInitialValues();
	}
}

void AWGasPlayerController::TeardownStaminaBar()
{
	StaminaWidgetController = nullptr;

	if (StaminaBarComponentInstance)
	{
		StaminaBarComponentInstance->DestroyComponent();
		StaminaBarComponentInstance = nullptr;
	}
}

void AWGasPlayerController::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	bool bIsDodging = false;
	if (UWGasAbilitySystemComponent* ASC = GetASC())
	{
		const FWGasGameplayTags& GameplayTags = FWGasGameplayTags::Get();
		UWGasMeleeAttack::SanitizeStaleAttackState(ASC);

		// 后摇有移动输入：淡出攻击 Montage，让走/跑 Locomotion 接管
		if (GameplayTags.State_Attacking_Recovery.IsValid()
			&& ASC->HasMatchingGameplayTag(GameplayTags.State_Attacking_Recovery)
			&& !InputAxisVector.IsNearlyZero())
		{
			if (UWGasMeleeAttack* Melee = UWGasMeleeAttack::GetActiveMeleeAttack(ASC))
			{
				Melee->CancelAttackForLocomotion();
			}
		}

		if (ASC->HasMatchingGameplayTag(GameplayTags.State_Attacking_Active))
		{
			return;
		}
		
		bIsDodging = ASC->HasMatchingGameplayTag(GameplayTags.State_Dodge);
	}
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	const AWGasCharacterHero* Hero = Cast<AWGasCharacterHero>(GetPawn());
	const bool bLockOnActive = Hero && Hero->GetLockOnComponent() && Hero->GetLockOnComponent()->IsLockedOn();

	if (ACharacter* GASCharacter = Cast<ACharacter>(GetPawn()))
	{
		// 锁定时相机朝 Boss，但不让角色跟着相机转；攻击时在 BeginMeleeAttack 里再转向
		if (InputAxisVector.Y > KINDA_SMALL_NUMBER && !bLockOnActive && !bIsDodging)
		{
			const FRotator TargetRotation(0.f, Rotation.Yaw, 0.f);
			const FRotator NewRotation = FMath::RInterpConstantTo(
				GASCharacter->GetActorRotation(),
				TargetRotation,
				GetWorld()->GetDeltaSeconds(),
				720.f);
			GASCharacter->SetActorRotation(NewRotation);
		}

		GASCharacter->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		GASCharacter->AddMovementInput(RightDirection, InputAxisVector.X);
	}
	if (!bIsDodging)
	{
		if (AWGasCharacterHero* HeroPawn = Cast<AWGasCharacterHero>(GetPawn()))
		{
			HeroPawn->UpdateRunningTag(InputAxisVector);
		}
	}
}

void AWGasPlayerController::StopMove(const FInputActionValue& InputActionValue)
{
	if (AWGasCharacterHero* Hero = Cast<AWGasCharacterHero>(GetPawn()))
	{
		Hero->ClearRunningTag();  
	}
}

void AWGasPlayerController::Look(const FInputActionValue& InputActionValue)
{
	if (const AWGasCharacterHero* Hero = Cast<AWGasCharacterHero>(GetPawn()))
	{
		if (Hero->GetLockOnComponent() && Hero->GetLockOnComponent()->IsLockedOn())
		{
			return;
		}
	}

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
		if (ASC->HasMatchingGameplayTag(GameplayTags.State_Attacking_Active)
			|| ASC->HasMatchingGameplayTag(GameplayTags.State_Dodge)
			|| ASC->HasMatchingGameplayTag(GameplayTags.State_Block))
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

void AWGasPlayerController::ToggleWalkRun(const FInputActionValue& InputActionValue)
{
	if (AWGasCharacterHero* Hero = Cast<AWGasCharacterHero>(GetPawn()))
	{
		Hero->ToggleWalkRun();
	}
}

void AWGasPlayerController::ToggleLockOnInput(const FInputActionValue& InputActionValue)
{
	if (AWGasCharacterHero* Hero = Cast<AWGasCharacterHero>(GetPawn()))
	{
		if (UWGasLockOnComponent* LockOn = Hero->FindComponentByClass<UWGasLockOnComponent>())
		{
			LockOn->ToggleLockOn();
		}
	}
}

UWGasAbilitySystemComponent* AWGasPlayerController::GetASC()
{
	if (WGasASC == nullptr)
	{
		WGasASC = Cast<UWGasAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn()));
	}
	return WGasASC;
}
