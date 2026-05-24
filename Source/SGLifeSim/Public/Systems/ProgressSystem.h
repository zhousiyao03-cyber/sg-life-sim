#pragma once

#include "CoreMinimal.h"

/** 某个软成就被解锁时广播（原生多播委托，纯 C++ 可用，不依赖 UObject）。 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAchievementUnlocked, FName /*AchievementId*/);

/**
 * 进度 / 软成就系统。spec §6.4。
 *
 * 纯 C++：用一个 TSet<FName> 记已达成的成就节点（第一笔 $10k、第一次升职…），
 * 零 UE GameplayFramework 依赖，方便单元测试。成就**只增不减**、去重。
 * 成就定义本身（标题/描述）后续走 DataTable，本系统只管「达成与否」。
 */
class SGLIFESIM_API FProgressSystem
{
public:
	/**
	 * 标记一个成就达成。
	 * @return true 表示这是**首次**达成（值得弹通知）；false 表示之前已达成。
	 */
	bool MarkAchieved(FName AchievementId);

	/** 查询某成就是否已达成。 */
	bool HasAchieved(FName AchievementId) const;

	/** 已达成成就数量。 */
	int32 GetAchievedCount() const { return Achieved.Num(); }

	/** 已达成集合（存档 / UI 用）。 */
	const TSet<FName>& GetAchievedSet() const { return Achieved; }

	/** 从存档恢复（清空后导入）。 */
	void RestoreAchieved(const TArray<FName>& Ids);

	/** 首次解锁时触发（订阅者：成就弹窗 / 音效）。 */
	FOnAchievementUnlocked OnAchievementUnlocked;

private:
	TSet<FName> Achieved;
};
