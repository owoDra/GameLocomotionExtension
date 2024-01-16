// Copyright (C) 2024 owoDra

using UnrealBuildTool;

public class GCLAddon : ModuleRules
{
	public GCLAddon(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[]
            {
                ModuleDirectory,
                ModuleDirectory + "/GCLAddon",
            }
        );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "ModularGameplay",
                "GameplayTags",
                "NetCore",
                "RigVM",
                "ControlRig",
                "AnimGraphRuntime",
            }
        );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "GFCore",
            }
        );

        SetupIrisSupport(Target);
    }
}
