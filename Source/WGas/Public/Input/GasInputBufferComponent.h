// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "GasInputBufferComponent.generated.h"

//该组件用于输入缓冲
USTRUCT()
struct FBufferedInput
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag InputTag;
	
	UPROPERTY()
	float TimerBuffered=0.f;
	
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WGAS_API UGasInputBufferComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UGasInputBufferComponent();


	UFUNCTION(BlueprintCallable,Category="Input")
	void BufferInput(FGameplayTag InputTag);

	UFUNCTION(BlueprintCallable,Category="Input")
	bool HasValidBuffer(FGameplayTag InputTag);

	UFUNCTION(BlueprintCallable,Category="Input")
	bool ConsumeBuffer(FGameplayTag InputTag);

	UFUNCTION(BlueprintCallable,Category="Input")
	void ClearBuffer(FGameplayTag InputTag);

	UFUNCTION(BlueprintCallable,Category="Input")
	void ClearAllBuffers();
	
	
protected:

	UPROPERTY(EditDefaultsOnly,Category="Input")
	float DefaultBufferDuration=0.3f;

	UPROPERTY()
	TArray<FBufferedInput> BufferedInputs;

	bool IsEntryValid(const FBufferedInput& Entry)const;
	
};
