#pragma once

#include "CoreMinimal.h"
#include "NightCommuteTypes.generated.h"

/**
 * 鬼月夜归抉择（Plan 23）。农历七月深夜回家，电梯自己停在 13 楼空楼层——
 * Uncle Lim 早就警告过：别进去，等下一趟。玩家在「贪图省事」与「招惹危险」间权衡。
 *
 * 这是第一个「玩家主动面对恐怖、承担选择后果」的桥段，区别于此前纯被动播报的恐怖事件。
 */
UENUM(BlueprintType)
enum class ENightCommuteChoice : uint8
{
	/** 等下一趟电梯：安全，但多耗时间/精力（守规矩）。 */
	WaitForNext   UMETA(DisplayName = "等下一趟（别进去）"),

	/** 赶紧进去：省事，但有概率招来恶性恐怖事件并重扣理智（犯禁忌）。 */
	StepIn        UMETA(DisplayName = "管它呢，赶紧进去"),

	/** 走楼梯：最稳妥，绕开电梯；很累（精力代价最大），但完全规避风险。 */
	TakeStairs    UMETA(DisplayName = "走楼梯上去"),
};

/** 一次夜归抉择的结算结果（纯数据，供 UI 播报）。 */
USTRUCT(BlueprintType)
struct FNightCommuteOutcome
{
	GENERATED_BODY()

	/** 结算文案（阴森或松一口气）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Horror")
	FText Message;

	/** 理智变化（负=扣）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Horror")
	int32 SanityDelta = 0;

	/** 精力变化（负=耗）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Horror")
	int32 EnergyDelta = 0;

	/** 是否真的出事了（犯禁忌且没躲过）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Horror")
	bool bSomethingHappened = false;
};
