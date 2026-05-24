#include "Systems/ShopSystem.h"

FShopItemDef FShopSystem::GetItemDef(EShopItem Item)
{
	FShopItemDef D;
	switch (Item)
	{
	case EShopItem::HotKopi:
		D.Title = FText::FromString(TEXT("好咖啡 Kopi-O"));
		D.PriceCents = 400; // $4
		D.AttrDelta[(int32)EPlayerAttribute::Energy] = 12;
		break;

	case EShopItem::Snacks:
		D.Title = FText::FromString(TEXT("零食"));
		D.PriceCents = 800; // $8
		D.AttrDelta[(int32)EPlayerAttribute::Mood] = 10;
		break;

	case EShopItem::WarmJacket:
		D.Title = FText::FromString(TEXT("保暖外套"));
		D.PriceCents = 6000; // $60
		D.AttrDelta[(int32)EPlayerAttribute::Health] = 14;
		break;

	case EShopItem::Amulet:
		D.Title = FText::FromString(TEXT("护身符"));
		D.PriceCents = 4000; // $40
		D.SanityDelta = 18; // 对抗恐惧螺旋
		break;

	case EShopItem::Count:
	default:
		break;
	}
	return D;
}
