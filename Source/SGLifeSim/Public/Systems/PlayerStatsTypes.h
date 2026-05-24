#pragma once

#include "CoreMinimal.h"
#include "PlayerStatsTypes.generated.h"

/**
 * 主角属性。spec §6.4，全部 0~100。
 * 用 uint8 是因为 UEnum + UPROPERTY 需要（Subsystem / 存档要暴露给 BP）。
 */
UENUM(BlueprintType)
enum class EPlayerAttribute : uint8
{
	Health        UMETA(DisplayName = "健康"),     // 影响能量上限 / 绩效 / 病假
	Mood          UMETA(DisplayName = "心情"),     // 影响关系 / 专注
	Energy        UMETA(DisplayName = "能量"),     // 每日恢复，活动消耗
	Professional  UMETA(DisplayName = "专业技能"),  // 主业绩效 / 跳槽
	Social        UMETA(DisplayName = "社交"),     // 关系建立速度
	Insight       UMETA(DisplayName = "见识"),     // 投资判断 / 解锁选项

	Count         UMETA(Hidden)  // 属性总数，用于遍历，不在 UI 显示
};
