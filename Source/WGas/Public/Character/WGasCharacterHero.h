// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/WGasCharacterBase.h"
#include "Input/GasInputBufferComponent.h"
#include "WGasCharacterHero.generated.h"

/**
 * 
 */

class UCameraComponent;
 
class USpringArmComponent;
class UGasInputBufferComponent;
UCLASS()
class WGAS_API AWGasCharacterHero : public AWGasCharacterBase
{
	GENERATED_BODY()
public:
	AWGasCharacterHero();
	virtual void PossessedBy(AController* NewController)override;

	
	UFUNCTION(BlueprintPure,Category="Input")
	UGasInputBufferComponent*GetWGasInputBufferComponent()const {return InputBufferComponent;}

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void ToggleWalkRun();

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsRunning() const { return bIsRunning; }

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float WalkSpeed = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float RunSpeed = 600.f;

	void ApplyMovementSpeed();

	TObjectPtr<UGasInputBufferComponent>InputBufferComponent;
	
	
	
private:

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent>CameraComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent>SpringArmComponent;


	bool bIsRunning = true;

	virtual void InitAbilityActorInfo() override;
};
