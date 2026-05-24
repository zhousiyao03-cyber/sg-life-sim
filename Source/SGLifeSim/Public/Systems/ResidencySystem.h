#pragma once

#include "CoreMinimal.h"
#include "Systems/ResidencyTypes.h"

/**
 * 居留身份系统。spec §6.4。
 *
 * 纯 C++ 状态机，零 UE 依赖、可单测。管身份阶梯的合法转换：
 * 工作准证(EP/SP) → 申请 PR → 通过(PR) / 被拒(退回原准证) → 入籍(公民)。
 */
class SGLIFESIM_API FResidencySystem
{
public:
	EResidencyStatus GetStatus() const { return Status; }

	/** PR 被拒的累计次数（终局判定用）。 */
	int32 GetPRRejectionCount() const { return PRRejectionCount; }

	/** 当前是否持工作准证（EP 或 SP）。 */
	bool IsOnWorkPermit() const;

	/**
	 * 申请 PR：仅持工作准证(EP/SP)时可申请，转入「申请中」。
	 * @return 成功发起返回 true；状态不允许返回 false。
	 */
	bool ApplyForPR();

	/**
	 * 裁决 PR 申请。仅在「申请中」有效。
	 * 通过 → PR；被拒 → 退回申请前的工作准证 + 被拒计数 +1。
	 * @return 有处理返回 true；非「申请中」返回 false。
	 */
	bool ResolvePRApplication(bool bApproved);

	/** 入籍：仅 PR 可入籍 → 公民。 */
	bool Naturalize();

	/** 从存档恢复。 */
	void RestoreState(EResidencyStatus InStatus, int32 InRejections);

private:
	EResidencyStatus Status = EResidencyStatus::WorkPermit_EP;
	/** 申请 PR 前的工作准证，用于被拒时退回。 */
	EResidencyStatus PriorPermit = EResidencyStatus::WorkPermit_EP;
	int32 PRRejectionCount = 0;
};
