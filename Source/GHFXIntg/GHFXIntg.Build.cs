// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GHFXIntg : ModuleRules
{
	public GHFXIntg(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
           new string[]
           {
                ModuleDirectory,
                ModuleDirectory + "/GHFXIntg",
           }
       );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "Niagara",
                "PhysicsCore",
                "GameplayTags",
            }
        );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "GFCore",
                "GCExt",
                "GFXExt",
                "GLExt",
                "GLHAddon",
            }
        );
    }
}
