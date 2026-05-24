#include "World/LocationRegistry.h"

FLocationDef FLocationRegistry::GetLocationDef(ELocation Location)
{
	FLocationDef D;
	switch (Location)
	{
	case ELocation::Rental:
		D.LevelName = FName(TEXT("L_Rental"));
		D.DisplayName = FText::FromString(TEXT("出租屋"));
		D.CityLocation = FVector(-1500.f, -1200.f, 0.f);
		D.Activities = { EActivityType::Sleep, EActivityType::Study, EActivityType::FreelanceCode,
			EActivityType::Exercise, EActivityType::PrayPuja };
		break;

	case ELocation::Hawker:
		D.LevelName = FName(TEXT("L_HawkerCenter"));
		D.DisplayName = FText::FromString(TEXT("食阁"));
		D.CityLocation = FVector(-200.f, -1400.f, 0.f);
		D.Activities = { EActivityType::EatHawker, EActivityType::Gossip };
		break;

	case ELocation::Office:
		D.LevelName = FName(TEXT("L_Office"));
		D.DisplayName = FText::FromString(TEXT("公司"));
		D.CityLocation = FVector(1600.f, 800.f, 0.f);
		// 上班暂用「接私活」代表伏案工作；专门的「上班」活动留后续。
		D.Activities = { EActivityType::FreelanceCode };
		break;

	case ELocation::Corridor:
		D.LevelName = FName(TEXT("L_Corridor"));
		D.DisplayName = FText::FromString(TEXT("组屋楼道"));
		D.CityLocation = FVector(-1500.f, 0.f, 0.f); // 出租屋楼附近
		// 楼道是过渡 / 恐怖触发点，无常规活动。
		D.Activities = {};
		break;

	case ELocation::MRT:
		D.LevelName = FName(TEXT("L_MRT"));
		D.DisplayName = FText::FromString(TEXT("地铁站"));
		D.CityLocation = FVector(400.f, 600.f, 0.f);
		D.Activities = {};
		break;

	case ELocation::Mall:
		D.LevelName = FName(TEXT("L_Mall"));
		D.DisplayName = FText::FromString(TEXT("商场"));
		D.CityLocation = FVector(1800.f, -400.f, 0.f);
		// 购物 / 消费活动留后续；先放「吃饭」（美食广场）。
		D.Activities = { EActivityType::EatHawker };
		break;

	case ELocation::None:
	default:
		break;
	}
	return D;
}

ELocation FLocationRegistry::FindLocationByLevel(const FString& LevelName)
{
	for (int32 i = 1; i < (int32)ELocation::Count; ++i)
	{
		const ELocation Loc = (ELocation)i;
		const FLocationDef Def = GetLocationDef(Loc);
		if (!Def.LevelName.IsNone() && LevelName.Contains(Def.LevelName.ToString()))
		{
			return Loc;
		}
	}
	return ELocation::None;
}

TArray<EActivityType> FLocationRegistry::GetActivitiesForLevel(const FString& LevelName)
{
	const ELocation Loc = FindLocationByLevel(LevelName);
	if (Loc == ELocation::None)
	{
		return {};
	}
	return GetLocationDef(Loc).Activities;
}
