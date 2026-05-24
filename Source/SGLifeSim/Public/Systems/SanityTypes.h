#pragma once

#include "CoreMinimal.h"
#include "SanityTypes.generated.h"

/**
 * 理智 / 恐惧状态档（恐怖玩法脊柱）。理智 0~100，越低越糟。
 * 见 docs/decisions/2026-05-24-first-person-horror-pivot.md。
 */
UENUM(BlueprintType)
enum class ESanityState : uint8
{
	Calm      UMETA(DisplayName = "平静"),       // >= 70
	Uneasy    UMETA(DisplayName = "不安"),       // 40-69
	Disturbed UMETA(DisplayName = "失常"),       // 15-39
	Breaking  UMETA(DisplayName = "濒临崩溃"),   // < 15
};
