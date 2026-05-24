// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SGLifeSim : ModuleRules
{
	public SGLifeSim(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
			"UMG",
		});

		// 启用 UE5 自动化测试（仅 Editor 构建）
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"UnrealEd",
				"AutomationController",
			});
		}
	}
}
