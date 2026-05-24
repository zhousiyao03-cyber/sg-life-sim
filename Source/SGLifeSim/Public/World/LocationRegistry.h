#pragma once

#include "CoreMinimal.h"
#include "World/LocationTypes.h"

/**
 * 地点注册表（开放城市枢纽，2026-05-24）。纯函数 / 零 UE 子系统依赖，可单测。
 * 游戏所有地点的单一事实源：城市建筑、关卡切换、活动过滤、NPC 填充都从这里读。
 * 新增地点 = 这里加一条 + 建一个 .umap 壳。
 */
class SGLIFESIM_API FLocationRegistry
{
public:
	/** 取某地点的定义。None / 无效返回空定义（LevelName 为 None）。 */
	static FLocationDef GetLocationDef(ELocation Location);

	/** 城市枢纽关卡名（玩家自由行走的大地图）。 */
	static FName GetCityLevelName() { return FName(TEXT("L_City")); }

	/** 关卡名 → 对应地点（含 PIE 前缀 / 路径子串匹配）。无匹配返回 None。 */
	static ELocation FindLocationByLevel(const FString& LevelName);

	/** 某关卡可做的活动（活动菜单过滤用）。非地点关卡返回空。 */
	static TArray<EActivityType> GetActivitiesForLevel(const FString& LevelName);
};
