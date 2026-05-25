// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class D1_Accessibility : ModuleRules
{
	public D1_Accessibility(ReadOnlyTargetRules Target) : base(Target)
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
			"SlateCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"D1_Accessibility",
			"D1_Accessibility/Variant_Platforming",
			"D1_Accessibility/Variant_Platforming/Animation",
			"D1_Accessibility/Variant_Combat",
			"D1_Accessibility/Variant_Combat/AI",
			"D1_Accessibility/Variant_Combat/Animation",
			"D1_Accessibility/Variant_Combat/Gameplay",
			"D1_Accessibility/Variant_Combat/Interfaces",
			"D1_Accessibility/Variant_Combat/UI",
			"D1_Accessibility/Variant_SideScrolling",
			"D1_Accessibility/Variant_SideScrolling/AI",
			"D1_Accessibility/Variant_SideScrolling/Gameplay",
			"D1_Accessibility/Variant_SideScrolling/Interfaces",
			"D1_Accessibility/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
