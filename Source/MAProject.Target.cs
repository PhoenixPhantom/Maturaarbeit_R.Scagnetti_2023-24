// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
//using System.Collections.Generic;

public class MAProjectTarget : TargetRules
{
	public MAProjectTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_2;
		ExtraModuleNames.Add("MAProject");
		
		if (bCompileAgainstEditor)
		{
			ExtraModuleNames.Add("MAProjectEditor");
		}
	}
}
