// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class WGas : ModuleRules
{
	public WGas(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"GameplayTasks",
			"GameplayTags",
			"GameplayAbilities",
			"EnhancedInput",
			"NavigationSystem"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"WGas",
			"WGas/Variant_Platforming",
			"WGas/Variant_Platforming/Animation",
			"WGas/Variant_Combat",
			"WGas/Variant_Combat/AI",
			"WGas/Variant_Combat/Animation",
			"WGas/Variant_Combat/Gameplay",
			"WGas/Variant_Combat/Interfaces",
			"WGas/Variant_Combat/UI",
			"WGas/Variant_SideScrolling",
			"WGas/Variant_SideScrolling/AI",
			"WGas/Variant_SideScrolling/Gameplay",
			"WGas/Variant_SideScrolling/Interfaces",
			"WGas/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
