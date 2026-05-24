#pragma once

#include "CoreMinimal.h"
#include "Systems/ActivityTypes.h"

/**
 * 活动系统。spec §6.1。
 *
 * 纯 C++：活动定义表 + 能量门槛判定，零 UE 依赖、可单测。
 * 「做什么活动有什么效果」在这里；「效果落到属性/钱/时间」由 UActivitySubsystem 接。
 */
class SGLIFESIM_API FActivitySystem
{
public:
	/** 某活动的定义（标题/时间块/属性&现金增减）。 */
	static FActivityDef GetActivityDef(EActivityType Activity);

	/**
	 * 是否做得动：做完能量不能低于 0。
	 * 恢复型活动（能量 delta ≥ 0，如睡觉/吃饭）总是可做。
	 */
	static bool CanPerform(const FActivityDef& Def, int32 CurrentEnergy);
};
