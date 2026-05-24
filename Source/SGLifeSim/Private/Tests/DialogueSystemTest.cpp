#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Systems/DialogueSystem.h"
#include "Systems/SGDialogueTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	// 造一棵小树：root 两个选项 —— A 无条件去 node_a；B 需好感≥50 去 node_b。
	FDialogueTree MakeTestTree()
	{
		FDialogueChoice ChoiceA;
		ChoiceA.Text = FText::FromString(TEXT("闲聊"));
		ChoiceA.NextNodeId = TEXT("node_a");
		ChoiceA.Effect.Type = EDialogueEffectType::AddAffinity;
		ChoiceA.Effect.Target = TEXT("AhHua");
		ChoiceA.Effect.Value = 5;

		FDialogueChoice ChoiceB;
		ChoiceB.Text = FText::FromString(TEXT("交心"));
		ChoiceB.NextNodeId = TEXT("node_b");
		ChoiceB.Condition.Type = EDialogueConditionType::MinAffinity;
		ChoiceB.Condition.Target = TEXT("AhHua");
		ChoiceB.Condition.Value = 50;

		FDialogueNode Root;
		Root.NodeId = TEXT("root");
		Root.Line = FText::FromString(TEXT("嗨。"));
		Root.Choices = { ChoiceA, ChoiceB };

		FDialogueNode NodeA;
		NodeA.NodeId = TEXT("node_a");
		NodeA.Line = FText::FromString(TEXT("天气不错。"));
		// node_a 一个结束选项
		FDialogueChoice Bye;
		Bye.Text = FText::FromString(TEXT("再见"));
		Bye.Effect.Type = EDialogueEffectType::EndDialogue;
		NodeA.Choices = { Bye };

		FDialogueNode NodeB;
		NodeB.NodeId = TEXT("node_b");
		NodeB.Line = FText::FromString(TEXT("其实我……"));

		FDialogueTree Tree;
		Tree.TreeId = TEXT("test");
		Tree.RootNodeId = TEXT("root");
		Tree.Nodes = { Root, NodeA, NodeB };
		return Tree;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDialogueStartTest,
	"SGLifeSim.Dialogue.StartsAtRoot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDialogueStartTest::RunTest(const FString& Parameters)
{
	FDialogueSystem Sys;
	TestFalse(TEXT("inactive before start"), Sys.IsActive());

	Sys.Start(MakeTestTree());
	TestTrue(TEXT("active after start"), Sys.IsActive());
	const FDialogueNode* Node = Sys.GetCurrentNode();
	TestNotNull(TEXT("current node valid"), Node);
	if (Node) { TestEqual(TEXT("at root"), Node->NodeId, FName(TEXT("root"))); }
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDialogueConditionGatingTest,
	"SGLifeSim.Dialogue.ConditionGatesChoices",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDialogueConditionGatingTest::RunTest(const FString& Parameters)
{
	FDialogueSystem Sys;
	Sys.Start(MakeTestTree());

	// 好感 = 10 → B(需50) 不可见，只有 A
	auto LowAffinity = [](const FDialogueCondition& C) { return 10 >= C.Value; };
	TArray<int32> Avail = Sys.GetAvailableChoiceIndices(LowAffinity);
	TestEqual(TEXT("only 1 choice at low affinity"), Avail.Num(), 1);
	TestEqual(TEXT("it is choice A (index 0)"), Avail[0], 0);

	// 好感 = 60 → A、B 都可见
	auto HighAffinity = [](const FDialogueCondition& C) { return 60 >= C.Value; };
	Avail = Sys.GetAvailableChoiceIndices(HighAffinity);
	TestEqual(TEXT("both choices at high affinity"), Avail.Num(), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDialogueChooseAdvancesAndEmitsEffectTest,
	"SGLifeSim.Dialogue.ChooseAdvancesAndEmitsEffect",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDialogueChooseAdvancesAndEmitsEffectTest::RunTest(const FString& Parameters)
{
	FDialogueSystem Sys;
	Sys.Start(MakeTestTree());
	auto AlwaysTrue = [](const FDialogueCondition&) { return true; };

	// 选 A → 产出加好感效果，跳到 node_a
	FDialogueEffect Eff;
	TestTrue(TEXT("choose A ok"), Sys.TryChoose(0, AlwaysTrue, Eff));
	TestEqual(TEXT("effect is AddAffinity"), Eff.Type, EDialogueEffectType::AddAffinity);
	TestEqual(TEXT("effect value 5"), Eff.Value, (int64)5);
	const FDialogueNode* Node = Sys.GetCurrentNode();
	TestNotNull(TEXT("now at node_a"), Node);
	if (Node) { TestEqual(TEXT("node_a"), Node->NodeId, FName(TEXT("node_a"))); }

	// node_a 的结束选项 → 结束对话
	FDialogueEffect Eff2;
	TestTrue(TEXT("choose bye ok"), Sys.TryChoose(0, AlwaysTrue, Eff2));
	TestFalse(TEXT("dialogue ended"), Sys.IsActive());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDialogueRejectsGatedChoiceTest,
	"SGLifeSim.Dialogue.RejectsGatedOrInvalidChoice",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDialogueRejectsGatedChoiceTest::RunTest(const FString& Parameters)
{
	FDialogueSystem Sys;
	Sys.Start(MakeTestTree());
	auto NeverMeets = [](const FDialogueCondition&) { return false; };

	// 选 B（被门控且条件不满足）→ 拒绝，状态不变
	FDialogueEffect Eff;
	TestFalse(TEXT("gated choice rejected"), Sys.TryChoose(1, NeverMeets, Eff));
	TestTrue(TEXT("still active"), Sys.IsActive());
	const FDialogueNode* Node = Sys.GetCurrentNode();
	if (Node) { TestEqual(TEXT("still at root"), Node->NodeId, FName(TEXT("root"))); }

	// 非法下标 → 拒绝
	TestFalse(TEXT("invalid index rejected"), Sys.TryChoose(99, NeverMeets, Eff));
	return true;
}

// 一个选项挂多个效果：新 TArray 重载产出全部，旧单效果重载只拿主效果（向后兼容）。
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDialogueMultiEffectTest,
	"SGLifeSim.Dialogue.ChoiceEmitsMultipleEffects",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDialogueMultiEffectTest::RunTest(const FString& Parameters)
{
	// root 一个选项：主效果加好感 +5，额外效果回理智 +12。
	FDialogueChoice C;
	C.Text = FText::FromString(TEXT("被理解"));
	C.Effect.Type = EDialogueEffectType::AddAffinity;
	C.Effect.Target = TEXT("UncleLim");
	C.Effect.Value = 5;
	FDialogueEffect San;
	San.Type = EDialogueEffectType::AddSanity;
	San.Value = 12;
	C.ExtraEffects.Add(San);

	// CollectEffects 直接验证：两个生效效果，顺序为主 → 额外。
	const TArray<FDialogueEffect> Collected = C.CollectEffects();
	TestEqual(TEXT("collects two effects"), Collected.Num(), 2);
	TestEqual(TEXT("first is affinity"), Collected[0].Type, EDialogueEffectType::AddAffinity);
	TestEqual(TEXT("second is sanity"), Collected[1].Type, EDialogueEffectType::AddSanity);

	FDialogueNode Root;
	Root.NodeId = TEXT("root");
	Root.Choices = { C };
	FDialogueTree Tree;
	Tree.TreeId = TEXT("multi");
	Tree.RootNodeId = TEXT("root");
	Tree.Nodes = { Root };

	auto AlwaysTrue = [](const FDialogueCondition&) { return true; };

	// 新重载：产出两个效果。
	FDialogueSystem Sys;
	Sys.Start(Tree);
	TArray<FDialogueEffect> Effects;
	TestTrue(TEXT("multi TryChoose ok"), Sys.TryChoose(0, AlwaysTrue, Effects));
	TestEqual(TEXT("two effects emitted"), Effects.Num(), 2);

	// 旧单效果重载：只拿主效果（向后兼容）。
	FDialogueSystem Sys2;
	Sys2.Start(Tree);
	FDialogueEffect Single;
	TestTrue(TEXT("single TryChoose ok"), Sys2.TryChoose(0, AlwaysTrue, Single));
	TestEqual(TEXT("single overload returns primary effect"), Single.Type, EDialogueEffectType::AddAffinity);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
