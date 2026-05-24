#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#include "Systems/HorrorSceneRegistry.h"
#include "Systems/HorrorSceneTypes.h"
#include "Systems/HorrorEventTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * 恐怖场景注册表纯核心（Plan 24）：每个场景有合法关卡名/理智代价>0/图鉴条目/事后文案；
 * 关卡名匹配能识别恐怖场景关卡（含 PIE 前缀子串）。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorSceneRegistryTest,
	"SGLifeSim.Horror.SceneRegistry",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHorrorSceneRegistryTest::RunTest(const FString& Parameters)
{
	// 每个非 None 场景的定义都完整。
	for (int32 i = 1; i < (int32)EHorrorScene::Count; ++i)
	{
		const EHorrorScene Scene = (EHorrorScene)i;
		const FHorrorSceneDef Def = FHorrorSceneRegistry::GetSceneDef(Scene);

		TestFalse(FString::Printf(TEXT("scene %d has a level name"), i), Def.LevelName.IsNone());
		TestTrue(FString::Printf(TEXT("scene %d sanity cost > 0"), i), Def.SanityCost > 0);
		TestNotEqual(FString::Printf(TEXT("scene %d maps to a codex entry"), i),
			Def.CodexEntry, EHorrorEvent::None);
		TestFalse(FString::Printf(TEXT("scene %d has aftermath text"), i), Def.AftermathText.IsEmpty());
	}

	// 电梯场景的具体映射。
	{
		const FHorrorSceneDef Elev = FHorrorSceneRegistry::GetSceneDef(EHorrorScene::Elevator);
		TestEqual(TEXT("elevator level name"), Elev.LevelName, FName(TEXT("L_ElevatorHorror")));
		TestEqual(TEXT("elevator sanity cost is 20"), Elev.SanityCost, 20);
		TestEqual(TEXT("elevator codex = ElevatorGhostFloor"), Elev.CodexEntry, EHorrorEvent::ElevatorGhostFloor);
	}

	// None 返回空定义。
	{
		const FHorrorSceneDef NoneDef = FHorrorSceneRegistry::GetSceneDef(EHorrorScene::None);
		TestTrue(TEXT("None scene has no level"), NoneDef.LevelName.IsNone());
	}

	// 关卡名识别（含 PIE 前缀 / 路径子串）。
	TestTrue(TEXT("plain elevator level recognized"),
		FHorrorSceneRegistry::IsHorrorSceneLevel(TEXT("L_ElevatorHorror")));
	TestTrue(TEXT("PIE-prefixed elevator level recognized"),
		FHorrorSceneRegistry::IsHorrorSceneLevel(TEXT("UEDPIE_0_L_ElevatorHorror")));
	TestFalse(TEXT("normal level not a horror scene"),
		FHorrorSceneRegistry::IsHorrorSceneLevel(TEXT("L_Rental")));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
