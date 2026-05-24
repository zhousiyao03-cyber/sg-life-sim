#pragma once

#include "CoreMinimal.h"
#include "TimeBlock.generated.h"

/**
 * 一天分成 5 个时间块。spec §5.2。
 * 用 uint8 是因为 UEnum + UPROPERTY 需要。
 */
UENUM(BlueprintType)
enum class ETimeBlock : uint8
{
	Morning     UMETA(DisplayName = "早"),
	Forenoon    UMETA(DisplayName = "上午"),
	Afternoon   UMETA(DisplayName = "下午"),
	Evening     UMETA(DisplayName = "晚"),
	LateNight   UMETA(DisplayName = "深夜"),
};

/** 一周 7 天。Singapore 习惯用周一为一周第一天。 */
UENUM(BlueprintType)
enum class EWeekday : uint8
{
	Monday      UMETA(DisplayName = "周一"),
	Tuesday     UMETA(DisplayName = "周二"),
	Wednesday   UMETA(DisplayName = "周三"),
	Thursday    UMETA(DisplayName = "周四"),
	Friday      UMETA(DisplayName = "周五"),
	Saturday    UMETA(DisplayName = "周六"),
	Sunday      UMETA(DisplayName = "周日"),
};
