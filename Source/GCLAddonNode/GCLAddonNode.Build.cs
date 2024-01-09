// Copyright (C) 2024 owoDra

using UnrealBuildTool;

public class GCLAddonNode : ModuleRules
{
	public GCLAddonNode(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[]
            {
                ModuleDirectory,
                ModuleDirectory + "/GCLAddonNode",
            }
        );

        PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"Core", 
				"CoreUObject", 
				"Engine", 
				"AnimationModifiers", 
				"AnimationBlueprintLibrary",
                "GCLAddon"
            }
		);

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(
				new[]
				{
					"AnimGraph",
					"AnimGraphRuntime",
					"BlueprintGraph"
				}
			);
		}
	}
}