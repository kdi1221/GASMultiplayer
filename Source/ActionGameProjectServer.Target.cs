// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ActionGameProjectServerTarget : TargetRules
{
	public ActionGameProjectServerTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("ActionGameProject");
		//ExtraModuleNames.AddRange( new string[] { "ActionGameProject", "ActionGameProjectEditor" } );
	}
}
