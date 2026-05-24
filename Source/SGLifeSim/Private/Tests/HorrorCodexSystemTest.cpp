#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#include "Systems/HorrorCodexSystem.h"
#include "Systems/HorrorEventTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * 恐怖图鉴纯核心（Plan 21）：标记/去重/计数/集齐/bitmask round-trip/None 不计。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCodexSystemTest,
	"SGLifeSim.Horror.CodexCollects",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHorrorCodexSystemTest::RunTest(const FString& Parameters)
{
	FHorrorCodexSystem Codex;

	// 开局空。
	TestEqual(TEXT("starts empty"), Codex.CountDiscovered(), 0);
	TestFalse(TEXT("not complete when empty"), Codex.IsComplete());
	TestEqual(TEXT("total = all non-None events"),
		FHorrorCodexSystem::TotalCollectable(), (int32)EHorrorEvent::Count - 1);

	// None 不计入。
	TestFalse(TEXT("None is not collectable"), Codex.MarkEncountered(EHorrorEvent::None));
	TestEqual(TEXT("still empty after None"), Codex.CountDiscovered(), 0);

	// 第一次标记返回 true，重复返回 false。
	TestTrue(TEXT("first mark is new"), Codex.MarkEncountered(EHorrorEvent::CorridorLights));
	TestFalse(TEXT("second mark not new"), Codex.MarkEncountered(EHorrorEvent::CorridorLights));
	TestEqual(TEXT("count is 1"), Codex.CountDiscovered(), 1);
	TestTrue(TEXT("has encountered it"), Codex.HasEncountered(EHorrorEvent::CorridorLights));
	TestFalse(TEXT("has not encountered another"), Codex.HasEncountered(EHorrorEvent::Pontianak));

	// 集齐：标记所有非 None 事件。
	for (int32 i = 1; i < (int32)EHorrorEvent::Count; ++i)
	{
		Codex.MarkEncountered((EHorrorEvent)i);
	}
	TestEqual(TEXT("all discovered"), Codex.CountDiscovered(), FHorrorCodexSystem::TotalCollectable());
	TestTrue(TEXT("complete after all"), Codex.IsComplete());

	// bitmask round-trip：导出后塞进新核心，状态一致。
	const int64 Mask = Codex.GetMask();
	FHorrorCodexSystem Restored;
	Restored.RestoreMask(Mask);
	TestEqual(TEXT("restored count matches"), Restored.CountDiscovered(), Codex.CountDiscovered());
	TestTrue(TEXT("restored has CorridorLights"), Restored.HasEncountered(EHorrorEvent::CorridorLights));
	TestTrue(TEXT("restored is complete"), Restored.IsComplete());

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
