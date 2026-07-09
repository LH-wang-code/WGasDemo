// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WGasEffectTypes.h"
#include "Components/ActorComponent.h"
#include "WGasCombatComponent.generated.h"


UCLASS( ClassGroup=(Combat), meta=(BlueprintSpawnableComponent) )
class WGAS_API UWGasCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UWGasCombatComponent();
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void BeginDamageWindow(const FDamagePayLoad& Payload);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EndDamageWindow();

	UFUNCTION(BlueprintCallable, Category = "Combat|WeaponSweep")
	void BeginWeaponSweep();

	UFUNCTION(BlueprintCallable, Category = "Combat|WeaponSweep")
	void EndWeaponSweep();
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|WeaponSweep")
	FName WeaponTipSocket = TEXT("Weapon_Tip");
	UPROPERTY(EditDefaultsOnly, Category = "Combat|WeaponSweep")
	FName WeaponBaseSocket = TEXT("Weapon_Base");

	UPROPERTY(EditDefaultsOnly, Category = "Combat|WeaponSweep")
	float WeaponSweepRadius = 25.f;
	UPROPERTY(EditDefaultsOnly, Category = "Combat|WeaponSweep")
	int32 WeaponSweepSampleCount = 10;
	UPROPERTY(EditDefaultsOnly, Category = "Combat|WeaponSweep")
	bool bDrawDebug = true;
private:
	FDamagePayLoad CurrentPayload;
	bool bDamageWindowActive = false;
	bool bWeaponSweepActive = false;

	FVector LastWeaponTipPos = FVector::ZeroVector;
	FVector LastWeaponBasePos = FVector::ZeroVector;
	TSet<TWeakObjectPtr<AActor>> HitActorsThisSwing;

	void TickWeaponSweep();
	USkeletalMeshComponent* GetTraceMesh() const;
	ACharacter* GetOwnerCharacter() const;
	void NotifyHit(AActor* HitActor);
};
