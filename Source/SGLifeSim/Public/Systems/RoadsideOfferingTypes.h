#pragma once

#include "CoreMinimal.h"
#include "RoadsideOfferingTypes.generated.h"

/**
 * 鬼月路边祭品抉择（Plan 26）。农历七月深夜回家，路中间有人摆了祭品、烧着金纸——
 * 本地人都知道的禁忌：别踩、别踢、别跨过去。玩家在「贪图省事」与「敬畏避让」间权衡。
 *
 * 与夜归电梯抉择（NightCommute）同构的第二个主动博弈，但第三选项语义不同：
 * 「拜一拜」是主动以敬畏化解恐惧（回理智），而非单纯绕路 —— 给两个博弈差异化的最优解。
 */
UENUM(BlueprintType)
enum class ERoadsideOfferingChoice : uint8
{
	/** 绕开走：多费点精力，但绝对安全（守规矩）。 */
	DetourAround  UMETA(DisplayName = "绕开走"),

	/** 直接跨过去：省事，但犯禁忌，有概率招事并重扣理智。 */
	StepOver      UMETA(DisplayName = "懒得绕，跨过去"),

	/** 停下拜一拜再走：本地人的智慧，最稳，敬畏化解恐惧回理智。 */
	PayRespects   UMETA(DisplayName = "停下，拜一拜再走"),
};

/** 一次路边祭品抉择的结算结果（纯数据，供 UI 播报）。 */
USTRUCT(BlueprintType)
struct FRoadsideOfferingOutcome
{
	GENERATED_BODY()

	/** 结算文案（阴森或安心）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Horror")
	FText Message;

	/** 理智变化（负=扣）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Horror")
	int32 SanityDelta = 0;

	/** 精力变化（负=耗）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Horror")
	int32 EnergyDelta = 0;

	/** 是否真的犯了禁忌招了事（跨过去且没躲过）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Horror")
	bool bSomethingHappened = false;
};
