#pragma once

#include "CoreMinimal.h"
#include "Systems/HorrorEventTypes.h"

/**
 * 恐怖图鉴纯逻辑核心（Plan 21）。零 UE 子系统依赖，可单测。
 *
 * 把「亲历过哪些恐怖事件」沉淀成可回看的收集——恐怖游戏经典的探索回报。
 * 用一个 int64 bitmask 记录（EHorrorEvent < 64，绰绰有余），bit 索引 = 枚举值。
 * None 不计入图鉴；可发现总数 = 所有非 None、非 Count 的事件数。
 */
class SGLIFESIM_API FHorrorCodexSystem
{
public:
	/** 标记某事件已亲历。None 忽略。首次发现返回 true。 */
	bool MarkEncountered(EHorrorEvent Event)
	{
		if (!IsCollectable(Event))
		{
			return false;
		}
		const int64 Bit = BitFor(Event);
		if ((Discovered & Bit) != 0)
		{
			return false; // 已发现过
		}
		Discovered |= Bit;
		return true;
	}

	/** 是否已亲历某事件。 */
	bool HasEncountered(EHorrorEvent Event) const
	{
		return IsCollectable(Event) && (Discovered & BitFor(Event)) != 0;
	}

	/** 已发现的事件数。 */
	int32 CountDiscovered() const
	{
		return FMath::CountBits((uint64)Discovered);
	}

	/** 图鉴里可发现的总条目数（所有非 None、非 Count 的事件）。 */
	static int32 TotalCollectable()
	{
		return (int32)EHorrorEvent::Count - 1; // 去掉 None
	}

	/** 是否已集齐全部。 */
	bool IsComplete() const
	{
		return CountDiscovered() >= TotalCollectable();
	}

	/** 原始 bitmask（存档用）。 */
	int64 GetMask() const { return Discovered; }

	/** 从存档回灌 bitmask（直接覆盖，不触发任何通知）。 */
	void RestoreMask(int64 InMask) { Discovered = InMask; }

private:
	/** 可收集 = 非 None、非 Count、在 [1, 63] 内。 */
	static bool IsCollectable(EHorrorEvent Event)
	{
		const int32 V = (int32)Event;
		return V > 0 && V < (int32)EHorrorEvent::Count && V < 64;
	}

	static int64 BitFor(EHorrorEvent Event)
	{
		return (int64)1 << (int32)Event;
	}

	int64 Discovered = 0;
};
