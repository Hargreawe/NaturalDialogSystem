// Copyright Epic Games, Inc. All Rights Reserved.

using System.Runtime.CompilerServices;
using UnrealBuildTool;

public class NaturalDialogSystem : ModuleRules
{
	public NaturalDialogSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", "Engine"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Projects",
				"CoreUObject",
				"Engine",
				"DeveloperSettings"
			}
		);
	}
}