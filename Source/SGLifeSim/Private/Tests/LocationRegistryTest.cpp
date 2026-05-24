#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#include "World/LocationRegistry.h"
#include "World/LocationTypes.h"
#include "Systems/ActivityTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * 地点注册表纯核心（开放城市枢纽）：每地点合法关卡名 / 城市坐标互不重叠 /
 * 活动是全集子集；关卡名匹配正确（含 PIE 前缀，城市枢纽不算地点）。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLocationRegistryTest,
	"SGLifeSim.World.LocationRegistry",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLocationRegistryTest::RunTest(const FString& Parameters)
{
	TArray<FVector> SeenCityLocs;
	TArray<FString> SeenLevels;

	for (int32 i = 1; i < (int32)ELocation::Count; ++i)
	{
		const ELocation Loc = (ELocation)i;
		const FLocationDef Def = FLocationRegistry::GetLocationDef(Loc);

		// 合法关卡名 + 显示名。
		TestFalse(FString::Printf(TEXT("location %d has level name"), i), Def.LevelName.IsNone());
		TestFalse(FString::Printf(TEXT("location %d has display name"), i), Def.DisplayName.IsEmpty());

		// 关卡名唯一。
		const FString LevelStr = Def.LevelName.ToString();
		TestFalse(FString::Printf(TEXT("location %d level unique"), i), SeenLevels.Contains(LevelStr));
		SeenLevels.Add(LevelStr);

		// 城市坐标互不重叠。
		bool bDup = false;
		for (const FVector& V : SeenCityLocs)
		{
			if (V.Equals(Def.CityLocation, 1.f)) { bDup = true; break; }
		}
		TestFalse(FString::Printf(TEXT("location %d city loc unique"), i), bDup);
		SeenCityLocs.Add(Def.CityLocation);

		// 活动都是合法枚举（< Count）。
		for (EActivityType A : Def.Activities)
		{
			TestTrue(FString::Printf(TEXT("location %d activity valid"), i),
				(int32)A < (int32)EActivityType::Count);
		}
	}

	// 具体映射。
	{
		const FLocationDef Rental = FLocationRegistry::GetLocationDef(ELocation::Rental);
		TestEqual(TEXT("rental level"), Rental.LevelName, FName(TEXT("L_Rental")));
		TestTrue(TEXT("rental can sleep"), Rental.Activities.Contains(EActivityType::Sleep));

		const FLocationDef Hawker = FLocationRegistry::GetLocationDef(ELocation::Hawker);
		TestTrue(TEXT("hawker can eat"), Hawker.Activities.Contains(EActivityType::EatHawker));
	}

	// 关卡名 → 地点匹配（含 PIE 前缀）。
	TestEqual(TEXT("L_Rental -> Rental"),
		FLocationRegistry::FindLocationByLevel(TEXT("L_Rental")), ELocation::Rental);
	TestEqual(TEXT("PIE-prefixed hawker -> Hawker"),
		FLocationRegistry::FindLocationByLevel(TEXT("UEDPIE_0_L_HawkerCenter")), ELocation::Hawker);

	// 城市枢纽 / 未知关卡不算地点。
	TestEqual(TEXT("city hub is not a location"),
		FLocationRegistry::FindLocationByLevel(TEXT("L_City")), ELocation::None);
	TestEqual(TEXT("unknown level -> None"),
		FLocationRegistry::FindLocationByLevel(TEXT("L_Nowhere")), ELocation::None);

	// 按关卡取活动。
	const TArray<EActivityType> HawkerActs = FLocationRegistry::GetActivitiesForLevel(TEXT("L_HawkerCenter"));
	TestTrue(TEXT("hawker level yields eat"), HawkerActs.Contains(EActivityType::EatHawker));
	TestEqual(TEXT("city hub yields no activities"),
		FLocationRegistry::GetActivitiesForLevel(TEXT("L_City")).Num(), 0);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
