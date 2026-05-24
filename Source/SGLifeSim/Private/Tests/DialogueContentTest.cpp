#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

#include "Systems/DialogueSystem.h"
#include "Systems/SGDialogueContent.h"
#include "Systems/SGDialogueTypes.h"
#include "Systems/DialogueSubsystem.h"
#include "Systems/RelationshipSubsystem.h"
#include "Systems/ProgressSubsystem.h"
#include "Systems/SGAchievementIds.h"

#if WITH_DEV_AUTOMATION_TESTS

// 所有正式对话树都必须通过完整性校验（无悬空节点引用）。
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDialogueContentValidatesTest,
	"SGLifeSim.Dialogue.ContentValidates",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDialogueContentValidatesTest::RunTest(const FString& Parameters)
{
	const TArray<FDialogueTree> Trees = SGDialogueContent::BuildAllTrees();
	TestTrue(TEXT("has content trees"), Trees.Num() >= 2);

	for (const FDialogueTree& Tree : Trees)
	{
		FString Error;
		const bool bValid = FDialogueSystem::ValidateTree(Tree, Error);
		TestTrue(FString::Printf(TEXT("tree '%s' valid: %s"), *Tree.TreeId.ToString(), *Error), bValid);
	}

	// 校验器能抓出悬空引用（防呆自检）。
	FDialogueTree Broken;
	Broken.TreeId = TEXT("broken");
	Broken.RootNodeId = TEXT("root");
	FDialogueNode Root;
	Root.NodeId = TEXT("root");
	FDialogueChoice Dangling;
	Dangling.Text = FText::FromString(TEXT("去不存在的节点"));
	Dangling.NextNodeId = TEXT("nope");
	Root.Choices = { Dangling };
	Broken.Nodes = { Root };
	FString Error;
	TestFalse(TEXT("dangling ref is caught"), FDialogueSystem::ValidateTree(Broken, Error));
	TestFalse(TEXT("error message present"), Error.IsEmpty());
	return true;
}

// AhHua 的「当年故事」深分支：好感够高才出现，选了解锁成就。
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDialogueStoryAchievementTest,
	"SGLifeSim.Integration.DialogueStoryAchievement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDialogueStoryAchievementTest::RunTest(const FString& Parameters)
{
	UGameInstance* GI = NewObject<UGameInstance>(GEngine);
	if (!GI) { return false; }
	GI->InitializeStandalone();

	UDialogueSubsystem*     Dlg  = GI->GetSubsystem<UDialogueSubsystem>();
	URelationshipSubsystem* Rel  = GI->GetSubsystem<URelationshipSubsystem>();
	UProgressSubsystem*     Prog = GI->GetSubsystem<UProgressSubsystem>();
	if (!Dlg || !Rel || !Prog) { GI->Shutdown(); return false; }

	// 好感低时，「当年故事」分支（需 70）不可见。
	TestTrue(TEXT("start AhHua"), Dlg->StartDialogue(TEXT("AhHua")));
	auto HasStoryChoice = [Dlg]()
	{
		for (const FText& T : Dlg->GetChoiceTexts())
		{
			if (T.ToString().Contains(TEXT("当年"))) { return true; }
		}
		return false;
	};
	TestFalse(TEXT("story hidden at low affinity"), HasStoryChoice());

	// 好感拉到 70 → 重开对话，故事分支可见。
	Rel->AddAffinity(TEXT("AhHua"), 70);
	TestTrue(TEXT("restart AhHua"), Dlg->StartDialogue(TEXT("AhHua")));
	TestTrue(TEXT("story visible at affinity 70"), HasStoryChoice());

	// 找到故事选项的可见下标并选它 → 解锁成就。
	int32 StoryIndex = INDEX_NONE;
	const TArray<FText> Choices = Dlg->GetChoiceTexts();
	for (int32 i = 0; i < Choices.Num(); ++i)
	{
		if (Choices[i].ToString().Contains(TEXT("当年"))) { StoryIndex = i; break; }
	}
	TestTrue(TEXT("found story choice"), StoryIndex != INDEX_NONE);
	if (StoryIndex != INDEX_NONE)
	{
		TestTrue(TEXT("choose story"), Dlg->ChooseOption(StoryIndex));
		TestTrue(TEXT("KnowNeighborStory unlocked"),
			Prog->HasAchieved(SGAchievementIds::KnowNeighborStory()));
	}

	GI->Shutdown();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
