#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

#include "Systems/DialogueSubsystem.h"
#include "Systems/HorrorCodexSubsystem.h"
#include "Systems/HorrorEventSubsystem.h"
#include "Systems/HorrorEventTypes.h"
#include "Systems/SanitySubsystem.h"
#include "Systems/RelationshipSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	/** 在当前节点的可见选项里找到含某子串的那一项的 visible index；找不到返回 -1。 */
	int32 FindChoiceContaining(const UDialogueSubsystem* Dlg, const TCHAR* Needle)
	{
		const TArray<FText> Texts = Dlg->GetChoiceTexts();
		for (int32 i = 0; i < Texts.Num(); ++i)
		{
			if (Texts[i].ToString().Contains(Needle))
			{
				return i;
			}
		}
		return -1;
	}
}

/**
 * 恐怖共鸣对话（Plan 22）：亲历过电梯空楼层后，Uncle Lim 才出现「坦白」分支；
 * 选它被理解后回一点理智。验证 HasDiscoveredHorror 门控 + AddSanity 效果。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorDialogueResonanceTest,
	"SGLifeSim.Integration.HorrorDialogueResonance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHorrorDialogueResonanceTest::RunTest(const FString& Parameters)
{
	UGameInstance* GI = NewObject<UGameInstance>(GEngine);
	if (!GI) { return false; }
	GI->InitializeStandalone();

	UDialogueSubsystem*     Dlg    = GI->GetSubsystem<UDialogueSubsystem>();
	UHorrorEventSubsystem*  Horror = GI->GetSubsystem<UHorrorEventSubsystem>();
	UHorrorCodexSubsystem*  Codex  = GI->GetSubsystem<UHorrorCodexSubsystem>();
	USanitySubsystem*       Sanity = GI->GetSubsystem<USanitySubsystem>();
	URelationshipSubsystem* Rel    = GI->GetSubsystem<URelationshipSubsystem>();
	if (!Dlg || !Horror || !Codex || !Sanity || !Rel) { GI->Shutdown(); return false; }

	// 未亲历电梯空楼层：开 Uncle Lim 对话，「坦白」分支不可见。
	TestTrue(TEXT("start UncleLim dialogue"), Dlg->StartDialogue(TEXT("UncleLim")));
	const int32 BeforeIdx = FindChoiceContaining(Dlg, TEXT("空楼层的电梯，我前几天真的碰到了"));
	TestEqual(TEXT("confide hidden before encounter"), BeforeIdx, -1);
	// 重新 StartDialogue 会重置到根节点，无需显式结束。

	// 亲历电梯空楼层（写进图鉴），并制造理智缺口。
	Horror->ApplyEvent(EHorrorEvent::ElevatorGhostFloor);
	TestTrue(TEXT("codex recorded elevator"), Codex->HasDiscovered(EHorrorEvent::ElevatorGhostFloor));
	Sanity->Drain(40); // 100 -> 60，留出回升空间
	const int32 SanityBefore = Sanity->GetSanity();

	// 再开对话：「坦白」分支应出现。
	TestTrue(TEXT("restart UncleLim dialogue"), Dlg->StartDialogue(TEXT("UncleLim")));
	const int32 ConfideIdx = FindChoiceContaining(Dlg, TEXT("空楼层的电梯，我前几天真的碰到了"));
	TestTrue(TEXT("confide visible after encounter"), ConfideIdx >= 0);

	// 选它进入 confide 节点，再选「踏实」——一个选项同时回理智 +12 与加好感 +6（多效果）。
	const int32 UncleAffBefore = Rel->GetAffinity(TEXT("UncleLim"));
	TestTrue(TEXT("choose confide"), Dlg->ChooseOption(ConfideIdx));
	const int32 ReassureIdx = FindChoiceContaining(Dlg, TEXT("踏实"));
	TestTrue(TEXT("reassure choice present"), ReassureIdx >= 0);
	TestTrue(TEXT("choose reassure"), Dlg->ChooseOption(ReassureIdx));

	TestEqual(TEXT("sanity restored by +12"), Sanity->GetSanity(), SanityBefore + 12);
	TestEqual(TEXT("affinity also raised +6 (multi-effect)"),
		Rel->GetAffinity(TEXT("UncleLim")), UncleAffBefore + 6);
	TestFalse(TEXT("dialogue ended after reassure"), Dlg->IsDialogueActive());

	// —— AhMei 共鸣（七月冥纸禁忌）：长辈安抚回理智 +10 ——
	// 未亲历：分支隐藏。
	TestTrue(TEXT("start AhMei dialogue"), Dlg->StartDialogue(TEXT("AhMei")));
	TestEqual(TEXT("AhMei confide hidden before encounter"),
		FindChoiceContaining(Dlg, TEXT("冥纸")), -1);

	Horror->ApplyEvent(EHorrorEvent::ZhiQianTaboo);
	TestTrue(TEXT("codex recorded zhiqian"), Codex->HasDiscovered(EHorrorEvent::ZhiQianTaboo));
	Sanity->Drain(40);
	const int32 AhMeiSanityBefore = Sanity->GetSanity();

	const int32 AhMeiAffBefore = Rel->GetAffinity(TEXT("AhMei"));
	TestTrue(TEXT("restart AhMei dialogue"), Dlg->StartDialogue(TEXT("AhMei")));
	const int32 AhMeiConfide = FindChoiceContaining(Dlg, TEXT("冥纸"));
	TestTrue(TEXT("AhMei confide visible after encounter"), AhMeiConfide >= 0);
	TestTrue(TEXT("choose AhMei confide"), Dlg->ChooseOption(AhMeiConfide));
	const int32 AhMeiReassure = FindChoiceContaining(Dlg, TEXT("安稳"));
	TestTrue(TEXT("AhMei reassure present"), AhMeiReassure >= 0);
	TestTrue(TEXT("choose AhMei reassure"), Dlg->ChooseOption(AhMeiReassure));
	// 一个选项同时回理智 +10 与加好感 +5（多效果）。
	TestEqual(TEXT("AhMei restored sanity +10"), Sanity->GetSanity(), AhMeiSanityBefore + 10);
	TestEqual(TEXT("AhMei affinity also raised +5 (multi-effect)"),
		Rel->GetAffinity(TEXT("AhMei")), AhMeiAffBefore + 5);

	// —— Wei 共鸣（末班地铁无倒影）：同龄人打岔回理智 +6 ——
	TestTrue(TEXT("start Wei dialogue"), Dlg->StartDialogue(TEXT("Wei")));
	TestEqual(TEXT("Wei confide hidden before encounter"),
		FindChoiceContaining(Dlg, TEXT("倒影")), -1);

	Horror->ApplyEvent(EHorrorEvent::MrtNoReflection);
	TestTrue(TEXT("codex recorded mrt"), Codex->HasDiscovered(EHorrorEvent::MrtNoReflection));
	Sanity->Drain(40);
	const int32 WeiSanityBefore = Sanity->GetSanity();

	TestTrue(TEXT("restart Wei dialogue"), Dlg->StartDialogue(TEXT("Wei")));
	const int32 WeiConfide = FindChoiceContaining(Dlg, TEXT("倒影"));
	TestTrue(TEXT("Wei confide visible after encounter"), WeiConfide >= 0);
	TestTrue(TEXT("choose Wei confide"), Dlg->ChooseOption(WeiConfide));
	const int32 WeiReassure = FindChoiceContaining(Dlg, TEXT("轻松"));
	TestTrue(TEXT("Wei reassure present"), WeiReassure >= 0);
	TestTrue(TEXT("choose Wei reassure"), Dlg->ChooseOption(WeiReassure));
	TestEqual(TEXT("Wei restored sanity +6"), Sanity->GetSanity(), WeiSanityBefore + 6);

	GI->Shutdown();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
