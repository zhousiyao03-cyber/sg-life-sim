#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#include "Systems/HorrorEventSystem.h"
#include "Systems/HorrorEventTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * 恐怖事件纯核心（Plan 15）：鬼月检测、鬼月限定门控、种子可复现、定义有文案。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorEventSystemTest,
	"SGLifeSim.Horror.PicksAndGates",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHorrorEventSystemTest::RunTest(const FString& Parameters)
{
	// 农历七月检测：第 7、19、31 个月是鬼月；1、8、12 不是。
	TestTrue(TEXT("month 7 is ghost month"), FHorrorEventSystem::IsGhostMonth(7));
	TestTrue(TEXT("month 19 is ghost month"), FHorrorEventSystem::IsGhostMonth(19));
	TestTrue(TEXT("month 31 is ghost month"), FHorrorEventSystem::IsGhostMonth(31));
	TestFalse(TEXT("month 1 not ghost"), FHorrorEventSystem::IsGhostMonth(1));
	TestFalse(TEXT("month 8 not ghost"), FHorrorEventSystem::IsGhostMonth(8));
	TestFalse(TEXT("month 12 not ghost"), FHorrorEventSystem::IsGhostMonth(12));

	// 非鬼月：抽很多次都不该出现鬼月限定事件。
	{
		FRandomStream Stream(12345);
		bool bSawGhostOnly = false;
		for (int32 i = 0; i < 2000; ++i)
		{
			const EHorrorEvent E = FHorrorEventSystem::PickEvent(Stream, /*bGhostMonth=*/false);
			if (FHorrorEventSystem::GetEventDef(E).bGhostMonthOnly)
			{
				bSawGhostOnly = true;
				break;
			}
		}
		TestFalse(TEXT("ghost-month-only events never picked outside ghost month"), bSawGhostOnly);
	}

	// 鬼月：抽很多次应能抽到鬼月限定事件（入池了）。
	{
		FRandomStream Stream(999);
		bool bSawGhostOnly = false;
		for (int32 i = 0; i < 2000; ++i)
		{
			const EHorrorEvent E = FHorrorEventSystem::PickEvent(Stream, /*bGhostMonth=*/true);
			if (FHorrorEventSystem::GetEventDef(E).bGhostMonthOnly)
			{
				bSawGhostOnly = true;
				break;
			}
		}
		TestTrue(TEXT("ghost-month-only events appear during ghost month"), bSawGhostOnly);
	}

	// 鬼月「无事」权重更低（更易出事）。
	TestTrue(TEXT("none weight lower in ghost month"),
		FHorrorEventSystem::GetNoneWeight(true) < FHorrorEventSystem::GetNoneWeight(false));

	// 同种子可复现。
	{
		FRandomStream A(7777), B(7777);
		bool bSame = true;
		for (int32 i = 0; i < 200; ++i)
		{
			if (FHorrorEventSystem::PickEvent(A, true) != FHorrorEventSystem::PickEvent(B, true))
			{
				bSame = false;
				break;
			}
		}
		TestTrue(TEXT("same seed -> same sequence"), bSame);
	}

	// 每个非 None 事件都有文案。
	for (int32 i = 1; i < (int32)EHorrorEvent::Count; ++i)
	{
		const FHorrorEventDef Def = FHorrorEventSystem::GetEventDef((EHorrorEvent)i);
		TestFalse(FString::Printf(TEXT("event %d has a title"), i), Def.Title.IsEmpty());
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
