#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

#include "Systems/DialogueSubsystem.h"
#include "Systems/RelationshipSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * 对话引擎端到端：headless GameInstance 上跑示例 AhHua 树，
 * 验证选项效果接到关系系统、好感门控的「交心」分支随好感解锁。Plan 5 Task 3。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDialogueIntegrationTest,
	"SGLifeSim.Integration.DialogueAffinityUnlock",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDialogueIntegrationTest::RunTest(const FString& Parameters)
{
	UGameInstance* GI = NewObject<UGameInstance>(GEngine);
	if (!GI) { return false; }
	GI->InitializeStandalone();

	UDialogueSubsystem*     Dlg = GI->GetSubsystem<UDialogueSubsystem>();
	URelationshipSubsystem* Rel = GI->GetSubsystem<URelationshipSubsystem>();
	if (!Dlg || !Rel) { GI->Shutdown(); return false; }

	// 开 AhHua 对话
	TestTrue(TEXT("start AhHua dialogue"), Dlg->StartDialogue(TEXT("AhHua")));
	TestTrue(TEXT("dialogue active"), Dlg->IsDialogueActive());

	// 初始好感 0 → 可见「闲聊」「送礼」「先走了」(都无条件)；「交心」(需50)/「问当年」(需70)/「报喜PR」(需身份) 被门控
	TestEqual(TEXT("3 visible choices at affinity 0"), Dlg->GetChoiceTexts().Num(), 3);

	// 选「送礼」(visible index 1) → 好感 +10
	TestTrue(TEXT("choose gift"), Dlg->ChooseOption(1));
	TestEqual(TEXT("affinity +10 after gift"), Rel->GetAffinity(TEXT("AhHua")), 10);
	TestFalse(TEXT("dialogue ended after gift->bye node? still active at gift node"),
		!Dlg->IsDialogueActive());  // 现在在 gift 节点，仍 active

	// gift 节点只有「先走了」→ 结束
	TestTrue(TEXT("choose bye"), Dlg->ChooseOption(0));
	TestFalse(TEXT("dialogue ended"), Dlg->IsDialogueActive());

	// 手动把好感拉到 50，再开对话 → 「交心」分支应解锁（闲聊/送礼/交心/先走了 = 4 个可见）
	Rel->AddAffinity(TEXT("AhHua"), 40);  // 10 + 40 = 50
	TestTrue(TEXT("restart dialogue"), Dlg->StartDialogue(TEXT("AhHua")));
	TestEqual(TEXT("4 visible choices at affinity 50"), Dlg->GetChoiceTexts().Num(), 4);

	GI->Shutdown();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
