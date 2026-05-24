#include "Systems/EconomicEventSystem.h"

FEconomicEventDef FEconomicEventSystem::GetEventDef(EEconomicEvent Event)
{
	switch (Event)
	{
	case EEconomicEvent::MarketRally:
		return FEconomicEventDef(NSLOCTEXT("SGEvents", "MarketRally", "股市回暖，你的持仓涨了 15%"),
			EEventEffectType::InvestmentReturnPerMille, 150, 8);
	case EEconomicEvent::MarketDip:
		return FEconomicEventDef(NSLOCTEXT("SGEvents", "MarketDip", "股市回调，你的持仓跌了 20%"),
			EEventEffectType::InvestmentReturnPerMille, -200, 6);
	case EEconomicEvent::CryptoBoom:
		return FEconomicEventDef(NSLOCTEXT("SGEvents", "CryptoBoom", "币圈大涨！你的持仓飙了 80%"),
			EEventEffectType::InvestmentReturnPerMille, 800, 2);
	case EEconomicEvent::CryptoCrash:
		return FEconomicEventDef(NSLOCTEXT("SGEvents", "CryptoCrash", "币圈崩盘，你的持仓腰斩 50%"),
			EEventEffectType::InvestmentReturnPerMille, -500, 3);
	case EEconomicEvent::YearEndBonus:
		return FEconomicEventDef(NSLOCTEXT("SGEvents", "YearEndBonus", "公司发年终奖，1.5 个月薪到账"),
			EEventEffectType::CashBonusSalaryMonthsX10, 15, 3);
	case EEconomicEvent::GovPayout:
		return FEconomicEventDef(NSLOCTEXT("SGEvents", "GovPayout", "政府发红包（GST voucher），现金 +$600"),
			EEventEffectType::CashDeltaCents, 60000, 4);
	case EEconomicEvent::BillShock:
		return FEconomicEventDef(NSLOCTEXT("SGEvents", "BillShock", "突发账单（看牙/修车），现金 -$400"),
			EEventEffectType::CashDeltaCents, -40000, 4);
	case EEconomicEvent::None:
	default:
		return FEconomicEventDef(FText::GetEmpty(), EEventEffectType::None, 0, 70);
	}
}

int32 FEconomicEventSystem::TotalWeight()
{
	int32 Total = 0;
	for (int32 i = 0; i < (int32)EEconomicEvent::Count; ++i)
	{
		Total += GetEventDef((EEconomicEvent)i).Weight;
	}
	return Total;
}

EEconomicEvent FEconomicEventSystem::PickEvent(FRandomStream& Stream)
{
	const int32 Total = TotalWeight();
	if (Total <= 0)
	{
		return EEconomicEvent::None;
	}
	// RandRange 含两端 → [0, Total-1]。
	int32 Roll = Stream.RandRange(0, Total - 1);
	for (int32 i = 0; i < (int32)EEconomicEvent::Count; ++i)
	{
		const int32 W = GetEventDef((EEconomicEvent)i).Weight;
		if (Roll < W)
		{
			return (EEconomicEvent)i;
		}
		Roll -= W;
	}
	return EEconomicEvent::None;
}
