#pragma once

#include "CoreMinimal.h"
#include "Systems/HorrorSceneTypes.h"

/**
 * 恐怖场景注册表（Plan 24）。纯函数 / 零 UE 子系统依赖，可单测。
 * 把 EHorrorScene 映射到其定义（关卡名 / 理智代价 / 图鉴条目 / 事后文案）。
 */
class SGLIFESIM_API FHorrorSceneRegistry
{
public:
	/** 取某场景的定义。None / 无效返回空定义（LevelName 为 None）。 */
	static FHorrorSceneDef GetSceneDef(EHorrorScene Scene);

	/** 关卡名 → 是否一个恐怖场景关卡（供进出判断）。 */
	static bool IsHorrorSceneLevel(const FString& LevelName);
};
