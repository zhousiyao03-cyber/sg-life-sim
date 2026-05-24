#pragma once

#include "CoreMinimal.h"
#include "EndingTypes.generated.h"

/**
 * 软终局。spec §6.5。没有 hard fail，只有四种「人生走向」。
 */
UENUM(BlueprintType)
enum class EEnding : uint8
{
	None        UMETA(DisplayName = "进行中"),
	Rooted      UMETA(DisplayName = "扎根"),       // PR + 房 + 稳定关系
	CashOut     UMETA(DisplayName = "兑现离开"),   // 攒够钱主动回国
	Heartbreak  UMETA(DisplayName = "心碎离开"),   // 续签失败 / 破产 / 关系破裂
	Adrift      UMETA(DisplayName = "留下但漂着"), // 没 PR、租房、关系薄
	Breakdown   UMETA(DisplayName = "被压垮"),     // 理智耗尽 / 精神崩溃（恐怖坏结局）
};
