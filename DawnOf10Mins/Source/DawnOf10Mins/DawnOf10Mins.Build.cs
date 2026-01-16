// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DawnOf10Mins : ModuleRules
{
	public DawnOf10Mins(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "NavigationSystem", "AIModule", "Niagara", "EnhancedInput" });
    }
}
