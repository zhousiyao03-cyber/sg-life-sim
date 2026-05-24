#pragma once

#include "CoreMinimal.h"
#include "Systems/PlayerStatsTypes.h"
#include "ActivityTypes.generated.h"

/** 时间块活动。spec §6.1。每个活动消耗时间块换属性/钱。 */
UENUM(BlueprintType)
enum class EActivityType : uint8
{
	Sleep        UMETA(DisplayName = "睡觉"),
	Study        UMETA(DisplayName = "学习"),
	FreelanceCode UMETA(DisplayName = "接私活（撸代码）"),
	Exercise     UMETA(DisplayName = "健身"),
	EatHawker    UMETA(DisplayName = "食阁吃饭"),
	Gossip       UMETA(DisplayName = "听八卦"),
	PrayPuja     UMETA(DisplayName = "拜拜祈福"),
	WorkShift    UMETA(DisplayName = "上班"),
	Shopping     UMETA(DisplayName = "逛街购物"),

	Count        UMETA(Hidden)
};

/**
 * 活动定义（纯数据）。属性增减用按 EPlayerAttribute 索引的数组（含能量），
 * 现金单列，时间块数单列。非 USTRUCT —— 纯 C++ 表，可单测。
 */
struct FActivityDef
{
	FText Title;

	/** 消耗的时间块数（推进 UTimeSubsystem N 次）。 */
	int32 TimeBlocks = 1;

	/** 各属性增减（索引 = (int32)EPlayerAttribute，含能量）。 */
	int32 AttrDelta[(int32)EPlayerAttribute::Count] = {};

	/** 现金增减（分，正入负出）。 */
	int64 CashDeltaCents = 0;

	/** 理智增减（恐怖玩法：拜拜/睡觉回理智）。 */
	int32 SanityDelta = 0;

	int32 GetAttr(EPlayerAttribute Attr) const { return AttrDelta[(int32)Attr]; }
};
