// Copyright (C) 2024 owoDra

using UnrealBuildTool;

public class GLExtNode : ModuleRules
{
	public GLExtNode(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[]
            {
                ModuleDirectory,
                ModuleDirectory + "/GLExtNode",
            }
        );

        PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"Core", "CoreUObject", "Engine", 

				"AnimationModifiers", "AnimationBlueprintLibrary",

                "GLExt"
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