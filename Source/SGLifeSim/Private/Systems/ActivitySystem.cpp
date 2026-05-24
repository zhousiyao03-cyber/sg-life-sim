#include "Systems/ActivitySystem.h"

namespace
{
	// 便于按属性名设置 delta 的小工具。
	void SetDelta(FActivityDef& Def, EPlayerAttribute Attr, int32 Value)
	{
		Def.AttrDelta[(int32)Attr] = Value;
	}
}

FActivityDef FActivitySystem::GetActivityDef(EActivityType Activity)
{
	FActivityDef Def;
	switch (Activity)
	{
	case EActivityType::Sleep:
		Def.Title = NSLOCTEXT("SGActivity", "Sleep", "睡觉（回满精力）");
		Def.TimeBlocks = 2;
		SetDelta(Def, EPlayerAttribute::Energy, 60);
		SetDelta(Def, EPlayerAttribute::Mood, 5);
		break;

	case EActivityType::Study:
		Def.Title = NSLOCTEXT("SGActivity", "Study", "学习（涨专业/见识）");
		Def.TimeBlocks = 1;
		SetDelta(Def, EPlayerAttribute::Professional, 4);
		SetDelta(Def, EPlayerAttribute::Insight, 2);
		SetDelta(Def, EPlayerAttribute::Energy, -15);
		break;

	case EActivityType::FreelanceCode:
		Def.Title = NSLOCTEXT("SGActivity", "Freelance", "接私活（赚 $300）");
		Def.TimeBlocks = 1;
		SetDelta(Def, EPlayerAttribute::Professional, 2);
		SetDelta(Def, EPlayerAttribute::Energy, -20);
		Def.CashDeltaCents = 30000; // +$300
		break;

	case EActivityType::Exercise:
		Def.Title = NSLOCTEXT("SGActivity", "Exercise", "健身（涨健康）");
		Def.TimeBlocks = 1;
		SetDelta(Def, EPlayerAttribute::Health, 5);
		SetDelta(Def, EPlayerAttribute::Mood, 3);
		SetDelta(Def, EPlayerAttribute::Energy, -10);
		break;

	case EActivityType::EatHawker:
		Def.Title = NSLOCTEXT("SGActivity", "Eat", "食阁吃饭（回精力）");
		Def.TimeBlocks = 1;
		SetDelta(Def, EPlayerAttribute::Energy, 20);
		SetDelta(Def, EPlayerAttribute::Mood, 5);
		SetDelta(Def, EPlayerAttribute::Health, 1);
		Def.CashDeltaCents = -500; // -$5
		break;

	case EActivityType::Gossip:
		Def.Title = NSLOCTEXT("SGActivity", "Gossip", "听八卦（涨见识/社交）");
		Def.TimeBlocks = 1;
		SetDelta(Def, EPlayerAttribute::Insight, 3);
		SetDelta(Def, EPlayerAttribute::Social, 3);
		SetDelta(Def, EPlayerAttribute::Mood, 2);
		SetDelta(Def, EPlayerAttribute::Energy, -5);
		break;

	default:
		break;
	}
	return Def;
}

bool FActivitySystem::CanPerform(const FActivityDef& Def, int32 CurrentEnergy)
{
	const int32 EnergyDelta = Def.GetAttr(EPlayerAttribute::Energy);
	if (EnergyDelta >= 0)
	{
		return true; // 恢复型总可做
	}
	return CurrentEnergy + EnergyDelta >= 0;
}
