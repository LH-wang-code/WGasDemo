// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "GameplayTagContainer.h"
#include "WGasPlayerController.generated.h"

/**
 * 
 */
class UInputMappingContext;
class UInputAction;
class UWGasInputConfig;
class UWGasAbilitySystemComponent;
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

private:
	void Move(const FInputActionValue& InputActionValue);

	void Look(const FInputActionValue& InputActionValue);

	void Jump(const FInputActionValue& InputActionValue);
	void StopJumping(const FInputActionValue& InputActionValue);

	UWGasAbilitySystemComponent* GetASC();
	
private:
	UPROPERTY()
	TObjectPtr<UWGasAbilitySystemComponent> WGasASC;
	
	UPROPERTY(EditAnywhere,Category="Input")
	TObjectPtr<UInputMappingContext>WGasContext;
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction>MoveAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction>LookAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction>JumpAction;


	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UWGasInputConfig>InputConfig;
};
