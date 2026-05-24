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

	/** 第一次升职。 */
	inline FName FirstPromotion() { return FName(TEXT("FirstPromotion")); }

	/** 听完邻居 Ah Hua 当年打拼的故事。 */
	inline FName KnowNeighborStory() { return FName(TEXT("KnowNeighborStory")); }

	/** 拿到同事 Wei 的内推机会。 */
	inline FName KnowColleague() { return FName(TEXT("KnowColleague")); }

	/** 第一次亲历都市传说（恐怖图鉴解锁第一条）。 */
	inline FName FirstUrbanLegend() { return FName(TEXT("FirstUrbanLegend")); }

	/** 集齐恐怖图鉴全部条目。 */
	inline FName CompleteHorrorCodex() { return FName(TEXT("CompleteHorrorCodex")); }

	/** 懂得敬畏：在七月禁忌博弈里做出守规矩的选择（等下一趟/走楼梯/绕开/拜一拜）。 */
	inline FName RespectTheUnseen() { return FName(TEXT("RespectTheUnseen")); }

	/** 净资产达成阈值（分）：$10,000。 */
	inline constexpr int64 NetWorth10kThresholdCents = 1000000;
}
