#pragma once

#include "CoreMinimal.h"
#include "Systems/HorrorEventTypes.h"

/**
 * 恐怖事件纯逻辑核心。零 UE 子系统依赖，可单测。
 * 加权随机选事件：鬼月限定事件只在农历七月入池，且鬼月里「无事」权重更低（更易出事）。
 */
class SGLIFESIM_API FHorrorEventSystem
{
public:
	/** 农历七月（鬼月）判定：把线性月号按 12 折算，第 7 个月即鬼月（7/19/31…）。 */
	static bool IsGhostMonth(int32 MonthNumber)
	{
		return MonthNumber > 0 && (((MonthNumber - 1) % 12) + 1 == 7);
	}

	/** 取某事件的定义。 */
	static FHorrorEventDef GetEventDef(EHorrorEvent Event);

	/** 「无事」的权重（鬼月更低 → 更易出事）。 */
	static int32 GetNoneWeight(bool bGhostMonth) { return bGhostMonth ? 35 : 75; }

	/**
	 * 加权随机选一个事件。bGhostMonth=false 时鬼月限定事件不入池。
	 * DreadBonus（低理智来的额外恐惧）从「无事」权重里扣，越大越容易出事。
	 * 注入 FRandomStream → 可复现。
	 */
	static EHorrorEvent PickEvent(FRandomStream& Stream, bool bGhostMonth, int32 DreadBonus = 0);
};
