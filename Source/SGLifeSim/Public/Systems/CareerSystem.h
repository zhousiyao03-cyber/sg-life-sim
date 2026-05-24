#pragma once

#include "CoreMinimal.h"
#include "Systems/CareerTypes.h"

/**
 * 职业/收入系统。spec §6.2。
 *
 * 纯 C++：当前职业等级 + 税前月薪（分）+ 在职月数，零 UE 依赖、可单测。
 * 「薪资是唯一真相」—— 不每次从等级反推，因为跳槽能让薪资高于本级基薪。
 * 升职靠专业技能 + 在职时长；跳槽按比例涨薪但资历清零。
 * 月薪如何发（CPF 分账等）由 Economy 负责；本核心只算「现在拿多少、能不能升」。
 */
class SGLIFESIM_API FCareerSystem
{
public:
	/** 默认从「初级工程师」起步（一个刚来新加坡的程序员）。 */
	FCareerSystem();

	ECareerLevel GetLevel() const { return Level; }
	int64 GetGrossSalaryCents() const { return GrossSalaryCents; }
	int32 GetMonthsInRole() const { return MonthsInRole; }
	bool IsEmployed() const { return Level != ECareerLevel::Unemployed; }

	/** 每月调用一次，累计在职时长（影响升职资历）。 */
	void AdvanceMonth() { ++MonthsInRole; }

	/** 下一等级（已是顶级则返回自身）。 */
	ECareerLevel NextLevel() const;

	/**
	 * 是否满足升职条件：未到顶 && 专业技能 ≥ 下一级门槛 && 在职 ≥ MinMonthsForPromotion。
	 */
	bool CanPromote(int32 Professional) const;

	/**
	 * 申请升职：满足条件则升一级，薪资 = max(当前薪资, 目标级基薪)，在职清零。
	 * 成功返回 true。
	 */
	bool Promote(int32 Professional);

	/**
	 * 跳槽：薪资 ×(1 + RaisePercent/100)，在职清零。待业不可跳槽。
	 * 成功返回 true。
	 */
	bool JobHop(int32 RaisePercent = DefaultJobHopRaisePercent);

	/** 从存档恢复。 */
	void RestoreState(ECareerLevel InLevel, int64 InGrossSalaryCents, int32 InMonthsInRole);

	/** 某等级的基础税前月薪（分）。 */
	static int64 BaseSalaryCents(ECareerLevel InLevel);

	/** 升「到」某等级所需的专业技能（0~100）。Junior/Unemployed 返回 0。 */
	static int32 PromotionProfessionalReq(ECareerLevel TargetLevel);

	/** 升职最低在职月数。 */
	static constexpr int32 MinMonthsForPromotion = 3;

	/** 跳槽默认涨幅（%）。spec §6.2「跳槽 +30~50%」取中位。 */
	static constexpr int32 DefaultJobHopRaisePercent = 35;

private:
	ECareerLevel Level = ECareerLevel::Junior;
	int64 GrossSalaryCents = 0;
	int32 MonthsInRole = 0;
};
