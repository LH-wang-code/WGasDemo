// Fill out your copyright notice in the Description page of Project Settings.


#include "WGasGameplayTags.h"
#include "GameplayTagsManager.h"
FWGasGameplayTags FWGasGameplayTags::GameplayTags;
void FWGasGameplayTags::InitializeNativeGameplayTags()
{

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


	GameplayTags.State_Attacking_Lighting = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("State.Attacking.Lighting"), FString("State.Attacking.Lighting"));  
	GameplayTags.State_Dodge = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("State.Dodge"), FString("State.Dodge"));  
	//PlayerBlock
	GameplayTags.Player_Block_InputPressed = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Player.Block.InputPressed"), FString("Player.Block.InputPressed"));
	GameplayTags.Player_Block_InputHeld = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Player.Block.InputHeld"), FString("Player.Block.InputHeld"));
	GameplayTags.Player_Block_InputReleased = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Player.Block.InputReleased"), FString("Player.Block.InputReleased"));
	GameplayTags.Player_Block_CursorTrace = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Player.Block.CursorTrace"), FString("Player.Block.CursorTrace"));

}
