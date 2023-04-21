// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MAProject : ModuleRules
{
	public MAProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"HeadMountedDisplay",
			"EnhancedInput",
			"MotionWarping"
		});
	}
}
