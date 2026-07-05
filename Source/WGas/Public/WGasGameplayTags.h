// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
/**
 * 
 */
struct FWGasGameplayTags
{

public:
	static const FWGasGameplayTags& Get(){return GameplayTags;}
	static void InitializeNativeGameplayTags();

	/*AttributeSet*/
	FGameplayTag Attribute_Vital_MaxHealth;
	FGameplayTag Attribute_Vital_MaxMana;
	FGameplayTag Attribute_Vital_MaxPoise;
	FGameplayTag Attribute_Vital_MaxStamina;
	
	
	



	/*InputTag*/
	FGameplayTag InputTag_LMB;
	FGameplayTag InputTag_RMB;
	FGameplayTag InputTag_1;
	FGameplayTag InputTag_2;
	FGameplayTag InputTag_3;
	FGameplayTag InputTag_4;
	FGameplayTag InputTag_Ctrl;

	/*State*/
	FGameplayTag State_Attacking_Lighting;
	FGameplayTag State_Dodge;

	FGameplayTag Player_Block_InputPressed;
	FGameplayTag Player_Block_InputHeld;
	FGameplayTag Player_Block_InputReleased;
	FGameplayTag Player_Block_CursorTrace;


	


	
private:
	static FWGasGameplayTags GameplayTags;
};
