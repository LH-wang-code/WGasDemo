#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "GameplayTagContainer.h"
#include "WGasPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UWGasInputConfig;
class UWGasAbilitySystemComponent;
class UStaminaBarComponent;
class UStaminaBarWGasWidgetController;
class UWGasStaminaBarWidget;

UCLASS()
class WGAS_API AWGasPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

private:
	void Move(const FInputActionValue& InputActionValue);
	void Look(const FInputActionValue& InputActionValue);
	void Jump(const FInputActionValue& InputActionValue);
	void StopJumping(const FInputActionValue& InputActionValue);
	void ToggleWalkRun(const FInputActionValue& InputActionValue);

	UWGasAbilitySystemComponent* GetASC();

	void SetupStaminaBar(APawn* InPawn);
	void BindStaminaBar();
	void TeardownStaminaBar();

	UPROPERTY()
	TObjectPtr<UWGasAbilitySystemComponent> WGasASC;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> WGasContext;
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> LookAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> ToggleWalkRunAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UWGasInputConfig> InputConfig;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Stamina")
	TSubclassOf<UStaminaBarComponent> StaminaBarComponentClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Stamina")
	TSubclassOf<UWGasStaminaBarWidget> StaminaBarWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Stamina")
	TSubclassOf<UStaminaBarWGasWidgetController> StaminaWidgetControllerClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Stamina")
	FVector StaminaBarRelativeLocation = FVector(0.f, 0.f, -90.f);

	UPROPERTY()
	TObjectPtr<UStaminaBarComponent> StaminaBarComponentInstance;

	UPROPERTY()
	TObjectPtr<UStaminaBarWGasWidgetController> StaminaWidgetController;
};
