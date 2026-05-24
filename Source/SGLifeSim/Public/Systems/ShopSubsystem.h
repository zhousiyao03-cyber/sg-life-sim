#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Systems/ShopSystem.h"
#include "Systems/ShopTypes.h"
#include "ShopSubsystem.generated.h"

/** 购买成功广播商品名（供 HUD 提示）。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPurchase, FText, ItemTitle);

/** 购买失败广播原因（如钱不够）。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPurchaseFailed, FText, Reason);

/**
 * 消费品商店子系统（Plan 25）。FShopSystem 的 UE 薄壳。
 *
 * 把购买落到 Economy（扣现金，余额不足即拒）/ PlayerState（加属性）/ Sanity（回理智）。
 * 即时消费品：买 = 当场结算，不进背包。商品表在纯 C++ 的 FShopSystem。
 */
UCLASS()
class SGLIFESIM_API UShopSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * 购买一件消费品：扣现金（不够则失败）→ 加属性 → 回理智 → 广播。
	 * 成功返回 true。
	 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Shop")
	bool TryPurchase(EShopItem Item);

	/** 当前现金是否买得起该商品（供 UI 灰显）。 */
	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Shop")
	bool CanAfford(EShopItem Item) const;

	UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Shop")
	FOnPurchase OnPurchase;

	UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Shop")
	FOnPurchaseFailed OnPurchaseFailed;
};
