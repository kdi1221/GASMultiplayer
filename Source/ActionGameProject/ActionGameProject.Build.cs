// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ActionGameProject : ModuleRules
{
	public ActionGameProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] 
		{ 
			"Core", 
			"CoreUObject", 
			"Engine",
			"EngineSettings",
			"InputCore", 
			"EnhancedInput",
            "GameplayAbilities",
            "MotionWarping",
            "NetCore",
            "MoviePlayer"
        });

		PrivateDependencyModuleNames.AddRange(new string[] 
		{
            "GameplayTags",
            "GameplayTasks",
            "AnimGraphRuntime",
			"Slate"
        });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
