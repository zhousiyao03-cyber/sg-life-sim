#pragma once

#include "CoreMinimal.h"
#include "Systems/PlayerStatsTypes.h"
#include "ShopTypes.generated.h"

/**
 * 即时消费品（Plan 25）。花钱买消费品，当场结算效果（不进背包）。
 * 给「钱」一个真正的去处；护身符回理智 —— 对抗恐惧螺旋的又一手段，呼应恐怖系统。
 * 见 docs/superpowers/specs/2026-05-24-consumable-shop-design.md。
 */
UENUM(BlueprintType)
enum class EShopItem : uint8
{
	HotKopi    UMETA(DisplayName = "好咖啡"),
	Snacks     UMETA(DisplayName = "零食"),
	WarmJacket UMETA(DisplayName = "保暖外套"),
	Amulet     UMETA(DisplayName = "护身符"),

	Count      UMETA(Hidden),
};

/**
 * 一个消费品的定义（纯数据，供 ShopSystem 查、可单测）。
 * 属性增减用按 EPlayerAttribute 索引的数组；理智单列（恐怖玩法）。
 */
USTRUCT(BlueprintType)
struct FShopItemDef
{
	GENERATED_BODY()

	/** 商品名（UI 显示）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Shop")
	FText Title;

	/** 价格（分，正值）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Shop")
	int64 PriceCents = 0;

	/** 各属性增减（索引 = (int32)EPlayerAttribute）。非 USTRUCT 数组，C++ 内部用。 */
	int32 AttrDelta[(int32)EPlayerAttribute::Count] = {};

	/** 理智增减（护身符等回理智）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Shop")
	int32 SanityDelta = 0;

	int32 GetAttr(EPlayerAttribute Attr) const { return AttrDelta[(int32)Attr]; }
};
