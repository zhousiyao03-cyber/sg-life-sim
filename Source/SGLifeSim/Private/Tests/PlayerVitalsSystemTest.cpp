#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#include "Systems/PlayerVitalsSystem.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * 玩家血量纯核心（B 块 GTA）：扣血/回血 clamp 在 0~100，归零即死。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayerVitalsSystemTest,
	"SGLifeSim.Combat.PlayerVitals",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerVitalsSystemTest::RunTest(const FString& Parameters)
{
	using V = FPlayerVitalsSystem;

	// 满血不算死。
	TestFalse(TEXT("full health alive"), V::IsDead(V::MaxHealth));

	// 扣血在区间内。
	TestEqual(TEXT("take 30 from full"), V::ApplyDamage(100, 30), 70);

	// 扣血不破底，归零即死。
	TestEqual(TEXT("overkill clamps to 0"), V::ApplyDamage(20, 999), 0);
	TestTrue(TEXT("zero health dead"), V::IsDead(0));

	// 负伤害无效（不回血）。
	TestEqual(TEXT("negative damage no-op"), V::ApplyDamage(50, -10), 50);

	// 回血不超上限。
	TestEqual(TEXT("heal capped at max"), V::Heal(80, 50), 100);
	TestEqual(TEXT("heal normal"), V::Heal(40, 25), 65);

	// 负回血无效。
	TestEqual(TEXT("negative heal no-op"), V::Heal(50, -10), 50);

	// clamp 边界。
	TestEqual(TEXT("clamp above"), V::Clamp(150), 100);
	TestEqual(TEXT("clamp below"), V::Clamp(-5), 0);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
