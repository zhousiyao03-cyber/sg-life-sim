#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#include "World/SGCityPopulatorSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * 城市装饰楼布局纯核心（开放城市枢纽）：确定性、装饰楼避开入口建筑、楼有正高度。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCityPopulatorLayoutTest,
	"SGLifeSim.World.CityLayout",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCityPopulatorLayoutTest::RunTest(const FString& Parameters)
{
	const TArray<FVector> Entrances = {
		FVector(-1500.f, -1200.f, 0.f),
		FVector(1600.f, 800.f, 0.f),
	};
	const float HalfExtent = 6000.f;
	const float Spacing = 900.f;
	const float ClearRadius = 700.f;

	const TArray<FDecorBuildingSpec> A = USGCityPopulatorSubsystem::BuildDecorLayout(
		Entrances, HalfExtent, Spacing, ClearRadius);

	// 生成了相当数量的装饰楼。
	TestTrue(TEXT("generates a city full of decor buildings"), A.Num() > 50);

	// 确定性：同输入同输出。
	const TArray<FDecorBuildingSpec> B = USGCityPopulatorSubsystem::BuildDecorLayout(
		Entrances, HalfExtent, Spacing, ClearRadius);
	TestEqual(TEXT("deterministic count"), A.Num(), B.Num());
	bool bSame = (A.Num() == B.Num());
	for (int32 i = 0; bSame && i < A.Num(); ++i)
	{
		if (!A[i].Location.Equals(B[i].Location, 0.1f) || !A[i].Scale.Equals(B[i].Scale, 0.1f))
		{
			bSame = false;
		}
	}
	TestTrue(TEXT("deterministic layout"), bSame);

	// 没有装饰楼落在入口建筑的清空半径内；且每栋楼高度为正。
	const float ClearSq = ClearRadius * ClearRadius;
	bool bAllClear = true;
	bool bAllPositiveHeight = true;
	for (const FDecorBuildingSpec& S : A)
	{
		for (const FVector& E : Entrances)
		{
			if (FVector::DistSquared2D(S.Location, E) < ClearSq) { bAllClear = false; break; }
		}
		if (S.Scale.Z <= 0.f) { bAllPositiveHeight = false; }
	}
	TestTrue(TEXT("no decor inside entrance clear radius"), bAllClear);
	TestTrue(TEXT("all decor have positive height"), bAllPositiveHeight);

	// 退化输入：spacing<=0 不崩、返回空。
	const TArray<FDecorBuildingSpec> Empty = USGCityPopulatorSubsystem::BuildDecorLayout(
		Entrances, HalfExtent, 0.f, ClearRadius);
	TestEqual(TEXT("zero spacing yields empty"), Empty.Num(), 0);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
