#pragma once

#include "CoreMinimal.h"

/**
 * 软成就 Id（spec §6.4）。集中定义，避免散落各处拼错。
 * 用 inline 函数返回 FName，规避静态初始化顺序问题（FName 子系统早于这些初始化）。
 */
namespace SGAchievementIds
{
	/** 第一次发薪（经济出现 Salary 流水）。 */
	inline FName FirstSalary() { return FName(TEXT("FirstSalary")); }

	/** 净资产首次达到 $10,000。 */
	inline FName NetWorth10k() { return FName(TEXT("NetWorth10k")); }

	/** 任一 NPC 好感首次达到「朋友」档。 */
	inline FName FirstFriend() { return FName(TEXT("FirstFriend")); }

	/** 净资产达成阈值（分）：$10,000。 */
	inline constexpr int64 NetWorth10kThresholdCents = 1000000;
}
