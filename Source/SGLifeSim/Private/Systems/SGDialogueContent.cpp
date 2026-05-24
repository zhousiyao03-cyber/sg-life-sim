#include "Systems/SGDialogueContent.h"
#include "Systems/SGAchievementIds.h"
#include "Systems/ResidencyTypes.h"

namespace
{
	// 便于构造选项的小工具。
	FDialogueChoice MakeChoice(const FString& Text, FName NextNodeId)
	{
		FDialogueChoice C;
		C.Text = FText::FromString(Text);
		C.NextNodeId = NextNodeId;
		return C;
	}

	FDialogueChoice MakeEndChoice(const FString& Text)
	{
		FDialogueChoice C;
		C.Text = FText::FromString(Text);
		C.Effect.Type = EDialogueEffectType::EndDialogue;
		return C;
	}

	FDialogueNode MakeNode(FName Id, const FText& Speaker, const FString& Line, TArray<FDialogueChoice> Choices)
	{
		FDialogueNode N;
		N.NodeId = Id;
		N.Speaker = Speaker;
		N.Line = FText::FromString(Line);
		N.Choices = MoveTemp(Choices);
		return N;
	}
}

FDialogueTree SGDialogueContent::BuildAhHuaTree()
{
	const FText Speaker = FText::FromString(TEXT("邻居 Ah Hua"));
	const FName Npc = TEXT("AhHua");

	// root 的几个选项（含好感/身份门控 + 效果）。
	FDialogueChoice Chat = MakeChoice(TEXT("随便聊两句"), TEXT("chat"));
	Chat.Effect.Type = EDialogueEffectType::AddAffinity;
	Chat.Effect.Target = Npc;
	Chat.Effect.Value = 3;

	FDialogueChoice Gift = MakeChoice(TEXT("送他一盒咖啡乌"), TEXT("gift"));
	Gift.Effect.Type = EDialogueEffectType::AddAffinity;
	Gift.Effect.Target = Npc;
	Gift.Effect.Value = 10;

	FDialogueChoice Heart = MakeChoice(TEXT("聊点掏心窝的"), TEXT("heart"));
	Heart.Condition.Type = EDialogueConditionType::MinAffinity;
	Heart.Condition.Target = Npc;
	Heart.Condition.Value = 50; // 朋友档解锁

	FDialogueChoice Story = MakeChoice(TEXT("问问他当年怎么熬过来的"), TEXT("story"));
	Story.Condition.Type = EDialogueConditionType::MinAffinity;
	Story.Condition.Target = Npc;
	Story.Condition.Value = 70; // 知己档解锁
	Story.Effect.Type = EDialogueEffectType::MarkAchievement;
	Story.Effect.Target = SGAchievementIds::KnowNeighborStory();

	FDialogueChoice PrReport = MakeChoice(TEXT("跟他说我拿到 PR 了"), TEXT("pr"));
	PrReport.Condition.Type = EDialogueConditionType::MinResidency;
	PrReport.Condition.Value = (int32)EResidencyStatus::PR;
	PrReport.Effect.Type = EDialogueEffectType::AddAffinity;
	PrReport.Effect.Target = Npc;
	PrReport.Effect.Value = 5;

	FDialogueTree Tree;
	Tree.TreeId = Npc;
	Tree.RootNodeId = TEXT("root");
	Tree.Nodes = {
		MakeNode(TEXT("root"), Speaker, TEXT("哟，又见面啦。最近过得怎么样？"),
			{ Chat, Gift, Heart, Story, PrReport, MakeEndChoice(TEXT("先走了")) }),

		MakeNode(TEXT("chat"), Speaker, TEXT("楼下那家鸡饭便宜又大碗，记得去试试。"),
			{ MakeChoice(TEXT("再聊会儿"), TEXT("root")), MakeEndChoice(TEXT("先走了")) }),

		MakeNode(TEXT("gift"), Speaker, TEXT("哎哟这么客气！下次来我家喝茶。"),
			{ MakeEndChoice(TEXT("不客气")) }),

		MakeNode(TEXT("heart"), Speaker, TEXT("说真的，刚来那几年我也天天怀疑自己是不是来错了地方。"),
			{ [Npc]() { FDialogueChoice C = MakeEndChoice(TEXT("安慰他几句")); C.Effect.Type = EDialogueEffectType::AddAffinity; C.Effect.Target = Npc; C.Effect.Value = 5; return C; }(),
			  MakeEndChoice(TEXT("我也是……")) }),

		MakeNode(TEXT("story"), Speaker, TEXT("我八几年从马来西亚过来，住得比你这还挤。熬呗，一年一年就过来了——这岛认人，也认命。"),
			{ MakeEndChoice(TEXT("谢谢你，Ah Hua")) }),

		MakeNode(TEXT("pr"), Speaker, TEXT("拿到 PR 啦？恭喜恭喜！这下算半个自己人咯。"),
			{ MakeEndChoice(TEXT("哈哈，谢谢")) }),
	};
	return Tree;
}

FDialogueTree SGDialogueContent::BuildAhMeiTree()
{
	const FText Speaker = FText::FromString(TEXT("食阁阿姨 Ah Mei"));
	const FName Npc = TEXT("AhMei");

	FDialogueChoice Buy = MakeChoice(TEXT("来一份鸡饭（$3.5）"), TEXT("eat"));
	Buy.Effect.Type = EDialogueEffectType::AddMoneyCents;
	Buy.Effect.Value = -350; // 花 $3.5

	FDialogueChoice Chat = MakeChoice(TEXT("跟她唠两句"), TEXT("chat"));
	Chat.Effect.Type = EDialogueEffectType::AddAffinity;
	Chat.Effect.Target = Npc;
	Chat.Effect.Value = 3;

	FDialogueTree Tree;
	Tree.TreeId = Npc;
	Tree.RootNodeId = TEXT("root");
	Tree.Nodes = {
		MakeNode(TEXT("root"), Speaker, TEXT("小伙子，吃什么？今天鸡饭特价。"),
			{ Buy, Chat, MakeEndChoice(TEXT("先不了")) }),

		MakeNode(TEXT("eat"), Speaker, TEXT("拿去，吃饱点。年轻人别老吃泡面，伤胃。"),
			{ MakeEndChoice(TEXT("谢谢阿姨")) }),

		MakeNode(TEXT("chat"), Speaker, TEXT("你们这些做 IT 的啊，天天加班到半夜，要顾着身体咧。"),
			{ MakeEndChoice(TEXT("知道啦")) }),
	};
	return Tree;
}

TArray<FDialogueTree> SGDialogueContent::BuildAllTrees()
{
	return { BuildAhHuaTree(), BuildAhMeiTree() };
}
