// Fill out your copyright notice in the Description page of Project Settings.


#include "WGasGameplayTags.h"
#include "GameplayTagsManager.h"
FWGasGameplayTags FWGasGameplayTags::GameplayTags;
void FWGasGameplayTags::InitializeNativeGameplayTags()
{

	GameplayTags.Attribute_Vital_MaxHealth=UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attribute.Vital.MaxHealth"),FString("Attribute.Vital.MaxHealth"));
	GameplayTags.Attribute_Vital_MaxMana=UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attribute.Vital.MaxMana"),FString("Attribute.Vital.MaxMana"));
	GameplayTags.Attribute_Vital_MaxPoise=UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attribute.Vital.MaxPoise"),FString("Attribute.Vital.MaxPoise"));
	GameplayTags.Attribute_Vital_MaxStamina=UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attribute.Vital.MaxStamina"),FString("Attribute.Vital.MaxStamina"));

	
	GameplayTags.InputTag_LMB=UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.LMB"),FString("Input Tag For Left Mouse Button"));
	GameplayTags.InputTag_RMB = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.RMB"), FString("Input Tag For Right Mouse Button"));  // 改为 RMB
	GameplayTags.InputTag_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.1"), FString("Input Tag For Key 1"));  // 改为 1
	GameplayTags.InputTag_2 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.2"), FString("Input Tag For Key 2"));  // 改为 2
	GameplayTags.InputTag_3 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.3"), FString("Input Tag For Key 3"));  // 改为 3
	GameplayTags.InputTag_4 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.4"), FString("Input Tag For Key 4"));  // 改为 4
	GameplayTags.InputTag_Ctrl = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.Ctrl"), FString("Input Tag For Key Ctrl"));  

	//Data
	GameplayTags.Data_Damage = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Data.Damage"), FString("Data.Damage"));
	GameplayTags.Data_PoiseDamage = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Data.PoiseDamage"), FString("Data.PoiseDamage"));

	GameplayTags.Abilities_Attack_Melee = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Attack.Melee"), FString("Abilities.Attack.Melee"));
	GameplayTags.Abilities_Dodge = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Dodge"), FString("Abilities.Dodge"));

	//Ability
	GameplayTags.Ability_Boss_Melee_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Ability.Boss.Melee.1"), FString("Ability.Boss.Melee.1"));
	GameplayTags.Ability_Boss_Melee_2 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Ability.Boss.Melee.2"), FString("Ability.Boss.Melee.2"));
	GameplayTags.Ability_Boss_Melee_3 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Ability.Boss.Melee.3"), FString("Ability.Boss.Melee.3"));
	GameplayTags.Ability_Boss_Melee_4 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Ability.Boss.Melee.4"), FString("Ability.Boss.Melee.4"));
	GameplayTags.Ability_Boss_Melee_5 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Ability.Boss.Melee.5"), FString("Ability.Boss.Melee.5"));
	GameplayTags.Ability_Boss_Charge = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Ability.Boss.Charge"), FString("Abilitiy.Boss.Charge"));
	GameplayTags.Ability_Player_Melee_Combo = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilitiy.Player.Melee.Combo"), FString("Abilitiy.Player.Melee.Combo"));

	//state
	GameplayTags.State_Attacking_Lighting = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.Attacking.Lighting"), FString("State.Attacking.Lighting"));
	GameplayTags.State_Attacking_Active = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.Attacking.Active"), FString("State.Attacking.Active"));
	GameplayTags.State_Attacking_Recovery = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.Attacking.Recovery"), FString("State.Attacking.Recovery"));
	GameplayTags.State_Dodge = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.Dodge"), FString("State.Dodge"));
	GameplayTags.State_Running = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.Running"), FString("State.Running"));
	GameplayTags.State_Block = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.Block"), FString("State.Block"));
	GameplayTags.State_Invulnerable = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.Invulnerable"), FString("State.Invulnerable"));
	GameplayTags.State_Boss_PoiseBroken = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.Boss.PoiseBroken"), FString("State.Boss.PoiseBroken"));
	GameplayTags.State_Boss_Invulnerable = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.Boss.Invulnerable"), FString("State.Boss.Invulnerable"));
	GameplayTags.State_Boss_PhaseTransition= UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.Boss.PhaseTransition"), FString("State.Boss.PhaseTransition"));
	GameplayTags.State_Boss_Phase_1= UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.Boss.Phase.1"), FString("State.Boss.Phase.1"));
	GameplayTags.State_Boss_Phase_2= UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.Boss.Phase.2"), FString("State.Boss.Phase.2"));

	//Event
	GameplayTags.Event_Boss_PoiseBroken = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.Boss.PoiseBroken"), FString("Event.Boss.PoiseBroken"));
	GameplayTags.Event_Attack_Active = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.Attack.Active"), FString("Event.Attack.Active"));
	GameplayTags.Event_Attack_Recovery = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.Attack.Recovery"), FString("Event.Attack.Recovery"));

	//Debuff
	GameplayTags.Debuff_Boss_Weak = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Debuff.Boss.Weak"), FString("Debuff.Boss.Weak"));

	//PlayerBlock
	GameplayTags.Player_Block_InputPressed = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Player.Block.InputPressed"), FString("Player.Block.InputPressed"));
	GameplayTags.Player_Block_InputHeld = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Player.Block.InputHeld"), FString("Player.Block.InputHeld"));
	GameplayTags.Player_Block_InputReleased = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Player.Block.InputReleased"), FString("Player.Block.InputReleased"));
	GameplayTags.Player_Block_CursorTrace = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Player.Block.CursorTrace"), FString("Player.Block.CursorTrace"));
	
	GameplayTags.State_Boss_Attacking = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.Boss.Attacking"), FString("Boss is performing an attack"));
}
