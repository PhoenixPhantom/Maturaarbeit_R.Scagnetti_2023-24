// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MAProject : ModuleRules
{
	public MAProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(new string[] {"Niagara", "NiagaraAnimNotifies", "Persona",
			"GenericGraphRuntime"});
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"PhysicsCore",
			"HeadMountedDisplay",
			"EnhancedInput",
			"MotionWarping",
			"UMG",
			"SlateCore",
			"GameplayTasks",
			"NavigationSystem",
			"AIModule",
			"SoundScape",
			"GameplayTags"
		});
	}
}
