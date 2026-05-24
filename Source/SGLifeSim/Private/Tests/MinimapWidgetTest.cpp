#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#include "UI/SGMinimapWidget.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * 小地图坐标映射纯核心：世界中心→地图中心、四角钳制、单调。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMinimapWidgetTest,
	"SGLifeSim.World.MinimapMapping",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMinimapWidgetTest::RunTest(const FString& Parameters)
{
	const float Mid = USGMinimapWidget::MapSize * 0.5f;

	// 世界原点 → 地图中心。
	const FVector2D Center = USGMinimapWidget::WorldToMap(FVector::ZeroVector);
	TestTrue(TEXT("world origin maps to map center"),
		FMath::IsNearlyEqual(Center.X, Mid, 0.5f) && FMath::IsNearlyEqual(Center.Y, Mid, 0.5f));

	// 超出范围被钳制在 [0, MapSize]。
	const FVector2D Far = USGMinimapWidget::WorldToMap(FVector(999999.f, -999999.f, 0.f));
	TestTrue(TEXT("X clamped to map"), Far.X >= 0.f && Far.X <= USGMinimapWidget::MapSize);
	TestTrue(TEXT("Y clamped to map"), Far.Y >= 0.f && Far.Y <= USGMinimapWidget::MapSize);
	TestTrue(TEXT("far +X maps to right edge"), FMath::IsNearlyEqual(Far.X, USGMinimapWidget::MapSize, 0.5f));
	TestTrue(TEXT("far -Y maps to top edge"), FMath::IsNearlyEqual(Far.Y, 0.f, 0.5f));

	// 单调：+X 世界坐标 → 更大的地图 X。
	const FVector2D A = USGMinimapWidget::WorldToMap(FVector(0.f, 0.f, 0.f));
	const FVector2D B = USGMinimapWidget::WorldToMap(FVector(1000.f, 0.f, 0.f));
	TestTrue(TEXT("greater world X -> greater map X"), B.X > A.X);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
