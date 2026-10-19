// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RustFall : ModuleRules
{
	public RustFall(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
