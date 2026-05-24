#pragma once

#include "CoreMinimal.h"
#include "Systems/ShopTypes.h"

/**
 * 消费品商店纯逻辑核心（Plan 25）。零 UE 子系统依赖，可单测。
 * 商品表是单一真相源；加商品只改 GetItemDef 一处。
 */
class SGLIFESIM_API FShopSystem
{
public:
	/** 某商品的定义（价格 / 属性效果 / 理智效果）。 */
	static FShopItemDef GetItemDef(EShopItem Item);

	/** 现金余额（分）是否买得起。供 UI 灰显买不起的按钮。 */
	static bool CanAfford(const FShopItemDef& Def, int64 BalanceCents)
	{
		return BalanceCents >= Def.PriceCents;
	}
};
