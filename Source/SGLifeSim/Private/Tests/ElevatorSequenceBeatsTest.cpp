#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#include "Systems/ElevatorSequenceBeats.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * 电梯演出节拍表纯核心（Plan 24）：时间单调递增、末节点是 Exit、总时长合理、含关键节拍。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FElevatorSequenceBeatsTest,
	"SGLifeSim.Horror.ElevatorBeats",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FElevatorSequenceBeatsTest::RunTest(const FString& Parameters)
{
	const TArray<FElevatorBeat> Beats = FElevatorSequenceBeats::GetBeats();

	TestTrue(TEXT("has beats"), Beats.Num() > 0);

	// 首节点是 Enter（0s）。
	TestEqual(TEXT("first beat is Enter"), (int32)Beats[0].Beat, (int32)EElevatorBeat::Enter);
	TestEqual(TEXT("first beat at t=0"), Beats[0].TimeSeconds, 0.f);

	// 时间单调非降。
	bool bMonotonic = true;
	for (int32 i = 1; i < Beats.Num(); ++i)
	{
		if (Beats[i].TimeSeconds < Beats[i - 1].TimeSeconds) { bMonotonic = false; break; }
	}
	TestTrue(TEXT("beat times monotonic non-decreasing"), bMonotonic);

	// 末节点是 Exit。
	TestEqual(TEXT("last beat is Exit"), (int32)Beats.Last().Beat, (int32)EElevatorBeat::Exit);

	// 总时长 = 末节点时间，且在合理区间（10~20s）。
	const float Dur = FElevatorSequenceBeats::GetTotalDuration();
	TestEqual(TEXT("duration equals last beat time"), Dur, Beats.Last().TimeSeconds);
	TestTrue(TEXT("duration within 10..20s"), Dur >= 10.f && Dur <= 20.f);

	// 含关键恐怖节拍：女鬼现身在女鬼消失之前。
	int32 RevealIdx = INDEX_NONE, GoneIdx = INDEX_NONE;
	for (int32 i = 0; i < Beats.Num(); ++i)
	{
		if (Beats[i].Beat == EElevatorBeat::GhostReveal) { RevealIdx = i; }
		if (Beats[i].Beat == EElevatorBeat::GhostGone)   { GoneIdx = i; }
	}
	TestTrue(TEXT("has GhostReveal"), RevealIdx != INDEX_NONE);
	TestTrue(TEXT("has GhostGone"), GoneIdx != INDEX_NONE);
	TestTrue(TEXT("ghost revealed before gone"), RevealIdx != INDEX_NONE && GoneIdx != INDEX_NONE && RevealIdx < GoneIdx);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
