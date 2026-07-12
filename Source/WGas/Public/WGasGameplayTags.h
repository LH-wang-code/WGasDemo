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

	//Data
	FGameplayTag Data_Damage;
	FGameplayTag Data_PoiseDamage;
	
	//Abilities

	FGameplayTag Abilities_Attack_Melee;
	FGameplayTag Abilities_Dodge;

	//Ability
	FGameplayTag Ability_Boss_Melee_1;
	FGameplayTag Ability_Boss_Melee_2;
	FGameplayTag Ability_Boss_Melee_3;
	FGameplayTag Ability_Boss_Melee_4;
	FGameplayTag Ability_Boss_Melee_5;
	
	FGameplayTag Ability_Boss_Charge;
	
	FGameplayTag Ability_Player_Melee_Combo;
	
	/*State*/
	FGameplayTag State_Attacking_Lighting;
	FGameplayTag State_Dodge;
	FGameplayTag State_Running;
	FGameplayTag State_Boss_Attacking;
	FGameplayTag State_Invulnerable;
	FGameplayTag State_Boss_PoiseBroken;
	

	/*
	 * Event
	 */
	FGameplayTag Event_Boss_PoiseBroken;


	/*
	 * Debuff
	 */
	FGameplayTag Debuff_Boss_Weak;
	
	FGameplayTag Player_Block_InputPressed;
	FGameplayTag Player_Block_InputHeld;
	FGameplayTag Player_Block_InputReleased;
	FGameplayTag Player_Block_CursorTrace;



	
private:
	static FWGasGameplayTags GameplayTags;
};
