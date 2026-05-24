#pragma once

#include "CoreMinimal.h"
#include "Systems/CareerTypes.h"
#include "Systems/ResidencyTypes.h"
#include "MilestoneTypes.generated.h"

/**
 * 人生里程碑（主线脊柱）。Plan 13。
 *
 * 一个外来程序员在新加坡「证明自己」的有序主线：从第一份薪水到成为公民。
 * 给沙盒一个方向感 —— 玩家随时知道「下一个该奔的目标是什么、还差多远」。
 * 顺序即枚举顺序（GetActive 取第一个未完成的）。
 */
UENUM(BlueprintType)
enum class EMilestone : uint8
{
	FirstSalary   UMETA(DisplayName = "站稳脚跟：拿到第一份薪水"),
	Save5k        UMETA(DisplayName = "有点积蓄：攒下 $5,000"),
	PromoteToMid  UMETA(DisplayName = "升职加薪：升上中级工程师"),
	BuyFirstHome  UMETA(DisplayName = "安身之所：买下自己的房"),
	BecomePR      UMETA(DisplayName = "落地生根：拿到 PR"),
	NetWorth100k  UMETA(DisplayName = "小有身家：净资产 $100,000"),
	BecomeCitizen UMETA(DisplayName = "这里是家：成为公民"),
	Count         UMETA(Hidden),
};

/**
 * 评估里程碑所需的状态快照（从各子系统聚合而来）。
 * 纯数据，让 FMilestoneSystem 可不依赖任何子系统单测。
 */
struct FMilestoneContext
{
	bool             bHasFirstSalary = false;
	int64            CashCents = 0;
	int64            NetWorthCents = 0;
	ECareerLevel     Career = ECareerLevel::Unemployed;
	bool             bOwnsHome = false;
	EResidencyStatus Residency = EResidencyStatus::WorkPermit_EP;
};

/** 单个里程碑的评估结果（是否达成 + 数值进度）。 */
struct FMilestoneProgress
{
	EMilestone Milestone = EMilestone::FirstSalary;
	bool  bComplete = false;
	bool  bIsNumeric = false;   // true 时 Current/Target 为金额（分）
	int64 CurrentCents = 0;
	int64 TargetCents = 0;
};
