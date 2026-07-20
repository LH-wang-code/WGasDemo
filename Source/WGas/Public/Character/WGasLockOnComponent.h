// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WGasLockOnComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType)
class WGAS_API UWGasLockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UWGasLockOnComponent();
	void ToggleLockOn();
	void UpdateLockOn(float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "LockOn")
	bool IsLockedOn() const { return bIsLockedOn; }

	UFUNCTION(BlueprintPure, Category = "LockOn")
	AActor* GetLockTarget() const { return LockTarget.Get(); }

	UFUNCTION(BlueprintPure, Category = "LockOn")
	FVector GetLockOnLocation(const AActor* Target) const;

	UFUNCTION(BlueprintPure, Category = "LockOn")
	FVector GetCurrentLockOnLocation() const;
protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	float MaxLockDistance=1500.f;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	float MaxLockHalfAngle=60;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	float LockInterpSpeed=12.f;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	float CharYawInterpSpeed=720.f;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FName LockOnSocketName=FName("Character1_Spine");
	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	float LockOnHeightOffset = 90.f; 
	
	
	virtual void BeginPlay() override;

private:
	bool bIsLockedOn = false;
	TWeakObjectPtr<AActor> LockTarget;

	AActor* FindBestTarget();
	void ClearLockOn();

	bool IsValidLockTarget(const AActor* Target) const;
};
