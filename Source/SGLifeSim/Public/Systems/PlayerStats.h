#pragma once

#include "CoreMinimal.h"
#include "Systems/PlayerStatsTypes.h"

/**
 * 主角属性数据核心。spec §6.4。
 *
 * 纯 C++：6 个 0~100 的属性，零 UE 依赖、可单测。clamp 在这里集中处理。
 * 被 UPlayerStateSubsystem 持有并暴露给 Blueprint。
 */
class SGLIFESIM_API FPlayerStats
{
public:
	static constexpr int32 MinValue = 0;
	static constexpr int32 MaxValue = 100;

	FPlayerStats();

	/** 读某属性。 */
	int32 Get(EPlayerAttribute Attr) const;

	/** 设某属性（clamp 到 [0,100]）。 */
	void Set(EPlayerAttribute Attr, int32 Value);

	/** 增减某属性（可负，结果 clamp）。 */
	void Modify(EPlayerAttribute Attr, int32 Delta);

	/** 每日恢复：能量回满。spec §6.4「能量每天恢复」。 */
	void RestoreEnergyDaily();

	/** 全部属性快照（存档用），索引 = (int32)EPlayerAttribute。 */
	TArray<int32> GetSnapshot() const;

	/** 从快照恢复（长度需为 Count，否则忽略）。 */
	void RestoreSnapshot(const TArray<int32>& Snapshot);

private:
	int32 Values[(int32)EPlayerAttribute::Count];
};
