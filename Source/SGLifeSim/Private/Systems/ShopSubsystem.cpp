#include "Systems/ShopSubsystem.h"

#include "Systems/EconomySubsystem.h"
#include "Systems/EconomyTypes.h"
#include "Systems/PlayerStateSubsystem.h"
#include "Systems/PlayerStatsTypes.h"
#include "Systems/SanitySubsystem.h"

bool UShopSubsystem::CanAfford(EShopItem Item) const
{
	const FShopItemDef Def = FShopSystem::GetItemDef(Item);
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const UEconomySubsystem* Eco = GI->GetSubsystem<UEconomySubsystem>())
		{
			return FShopSystem::CanAfford(Def, Eco->GetBalance(ECurrencyAccount::Cash));
		}
	}
	return false;
}

bool UShopSubsystem::TryPurchase(EShopItem Item)
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) { return false; }

	const FShopItemDef Def = FShopSystem::GetItemDef(Item);

	// 扣钱：TryWithdraw 余额不足返回 false 且不改动任何状态 —— 一步完成判钱 + 扣钱。
	UEconomySubsystem* Eco = GI->GetSubsystem<UEconomySubsystem>();
	if (!Eco || !Eco->TryWithdraw(ECurrencyAccount::Cash, Def.PriceCents, TEXT("Shop")))
	{
		OnPurchaseFailed.Broadcast(FText::FromString(TEXT("钱不够。")));
		return false;
	}

	// 加属性。
	if (UPlayerStateSubsystem* PS = GI->GetSubsystem<UPlayerStateSubsystem>())
	{
		for (int32 i = 0; i < (int32)EPlayerAttribute::Count; ++i)
		{
			const int32 Delta = Def.AttrDelta[i];
			if (Delta != 0)
			{
				PS->ModifyAttribute((EPlayerAttribute)i, Delta);
			}
		}
	}

	// 回理智（护身符等 —— 对抗恐惧螺旋）。
	if (Def.SanityDelta > 0)
	{
		if (USanitySubsystem* San = GI->GetSubsystem<USanitySubsystem>())
		{
			San->Restore(Def.SanityDelta);
		}
	}

	OnPurchase.Broadcast(Def.Title);
	return true;
}
