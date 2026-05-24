#pragma once

#include "CoreMinimal.h"
#include "Math/RandomStream.h"
#include "Systems/EconomicEventTypes.h"

/**
 * 随机经济事件系统。spec §6.2。
 *
 * 纯 C++：事件定义表 + 注入 FRandomStream 的加权抽取，零 UE 依赖、可确定性单测。
 * 「抽什么」在这里（确定性可复现），「效果怎么落到钱包/持仓」由 UEconomicEventSubsystem 接。
 */
class SGLIFESIM_API FEconomicEventSystem
{
public:
	/** 某事件的定义（标题/效果/权重）。 */
	static FEconomicEventDef GetEventDef(EEconomicEvent Event);

	/** 所有事件权重之和（含 None）。 */
	static int32 TotalWeight();

	/** 用给定随机流按权重抽一个事件（多数月返回 None）。会消费 Stream 状态。 */
	static EEconomicEvent PickEvent(FRandomStream& Stream);
};
