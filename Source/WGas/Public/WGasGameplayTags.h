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
	FGameplayTag Attribute_Vital_MaxMomentum;
	
	
	
	


	/*InputTag*/
	FGameplayTag InputTag_LMB;
	FGameplayTag InputTag_RMB;
	FGameplayTag InputTag_1;
	FGameplayTag InputTag_2;
	FGameplayTag InputTag_3;
	FGameplayTag InputTag_4;
	FGameplayTag InputTag_Ctrl;
	FGameplayTag InputTag_R;
	
	//Data
	FGameplayTag Data_Damage;
	FGameplayTag Data_PoiseDamage;
	
	//Abilities

	FGameplayTag Abilities_Attack_Melee;
	FGameplayTag Abilities_Dodge;
	FGameplayTag Abilities_Momentum_Ultimate;
	

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
	FGameplayTag State_Attacking_Active;
	FGameplayTag State_Attacking_Recovery;
	
	FGameplayTag State_Dodge;
	FGameplayTag State_Running;
	FGameplayTag State_HitReact; 
	FGameplayTag State_Block;
	FGameplayTag State_Parry;
	FGameplayTag State_Parry_Window;
	FGameplayTag State_Parry_Success;
	FGameplayTag State_Momentum_Full;
	FGameplayTag State_Boss_Attacking;
	FGameplayTag State_Invulnerable;
	FGameplayTag State_Boss_PoiseBroken;
	FGameplayTag State_Boss_Invulnerable;
	FGameplayTag State_Boss_PhaseTransition;
	FGameplayTag State_Boss_Phase_1;
	FGameplayTag State_Boss_Phase_2;
	FGameplayTag State_Parried;
	FGameplayTag State_Dead;
	/*
	 * Event
	 */
	FGameplayTag Event_Boss_PoiseBroken;
	FGameplayTag Event_Attack_Active;
	FGameplayTag Event_Attack_Recovery;

	/*
	 * Debuff
	 */
	FGameplayTag Debuff_Boss_Weak;
	FGameplayTag Debuff_MomentumUltimate_Bleed;
	FGameplayTag Debuff_MomentumUltimate_Vulnerable;
	FGameplayTag Debuff_MomentumUltimate_PoiseBreak;
	
	FGameplayTag Player_Block_InputPressed;
	FGameplayTag Player_Block_InputHeld;
	FGameplayTag Player_Block_InputReleased;
	FGameplayTag Player_Block_CursorTrace;

	/*
	 * Ultimation
	 * 
	 */
	
	FGameplayTag Mark_MomentumULtimate_Layer_1;
	FGameplayTag Mark_MomentumULtimate_Layer_2;
	FGameplayTag Mark_MomentumULtimate_Layer_3;
	
private:
	static FWGasGameplayTags GameplayTags;
};
