#pragma once

#include "CoreMinimal.h"
#include "Systems/TimeBlock.h"

/**
 * 时间系统。spec §6.1。
 *
 * 纯数据 + 推进逻辑，零 UE GameplayFramework 依赖，方便单元测试。
 * 后续会被 Blueprint 包装的 GameInstanceSubsystem 持有（见 UTimeSubsystem）。
 *
 * 内部用 int32 TotalBlocksSinceStart 单一来源，所有派生量（当前 block / 周几 /
 * 第几天）都从它算出来 —— 避免多状态同步 bug。
 */
class SGLIFESIM_API FTimeSystem
{
public:
	FTimeSystem();

	/** 推进一个时间块。可能跨天 / 跨周。 */
	void AdvanceBlock();

	/** 当前所处的时间块。 */
	ETimeBlock GetCurrentBlock() const;

	/** 当前是游戏内的第几天（从 1 开始）。 */
	int32 GetDayNumber() const;

	/** 当前是周几。 */
	EWeekday GetWeekday() const;

	/** 当前是第几个月（从 1 开始）。spec §5.3 月度账单/发薪用。 */
	int32 GetMonthNumber() const;

	/** 当前是本月第几天（从 1 开始）。== 1 即「每月 1 号」。 */
	int32 GetDayOfMonth() const;

	/** 自游戏开始累计的总时间块数（测试用 / 存档用）。 */
	int32 GetTotalBlocks() const { return TotalBlocksSinceStart; }

private:
	/** 单一来源。所有派生量从这里算。 */
	int32 TotalBlocksSinceStart = 0;

	static constexpr int32 BlocksPerDay = 5;
	static constexpr int32 DaysPerWeek = 7;
	// 一个月 = 4 周（28 天），让每月固定从周一开始，月度数学干净。spec §5.3。
	static constexpr int32 DaysPerMonth = 28;
};
