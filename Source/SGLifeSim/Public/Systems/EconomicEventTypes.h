#pragma once

#include "CoreMinimal.h"
#include "EconomicEventTypes.generated.h"

/** 随机经济事件。spec §6.2。None = 本月平静（权重最高）。 */
UENUM(BlueprintType)
enum class EEconomicEvent : uint8
{
	None        UMETA(DisplayName = "平静"),
	MarketRally UMETA(DisplayName = "股市回暖"),
	MarketDip   UMETA(DisplayName = "股市回调"),
	CryptoBoom  UMETA(DisplayName = "币圈大涨"),
	CryptoCrash UMETA(DisplayName = "币圈崩盘"),
	YearEndBonus UMETA(DisplayName = "年终奖"),
	GovPayout   UMETA(DisplayName = "政府红包"),
	BillShock   UMETA(DisplayName = "突发账单"),

	Count       UMETA(Hidden)
};

/** 事件效果类型。 */
UENUM(BlueprintType)
enum class EEventEffectType : uint8
{
	None                     UMETA(DisplayName = "无"),
	InvestmentReturnPerMille UMETA(DisplayName = "投资涨跌(千分比)"),
	CashDeltaCents           UMETA(DisplayName = "现金增减(分)"),
	CashBonusSalaryMonthsX10 UMETA(DisplayName = "现金奖金(月薪×0.1)"),
};

/** 一个经济事件的定义：标题 + 效果 + 抽取权重。 */
USTRUCT(BlueprintType)
struct FEconomicEventDef
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	FText Title;

	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	EEventEffectType EffectType = EEventEffectType::None;

	/** 效果幅度，含义随 EffectType：投资千分比 / 现金分 / 月薪×0.1 的份数。 */
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	int32 Magnitude = 0;

	/** 月度加权抽取权重（越大越常见）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	int32 Weight = 0;

	FEconomicEventDef() = default;
	FEconomicEventDef(const FText& InTitle, EEventEffectType InType, int32 InMagnitude, int32 InWeight)
		: Title(InTitle), EffectType(InType), Magnitude(InMagnitude), Weight(InWeight) {}
};
