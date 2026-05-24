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

FDialogueTree SGDialogueContent::BuildUncleLimTree()
{
	const FText Speaker = FText::FromString(TEXT("保安 Uncle Lim"));
	const FName Npc = TEXT("UncleLim");

	FDialogueChoice Chat = MakeChoice(TEXT("打个招呼"), TEXT("chat"));
	Chat.Effect.Type = EDialogueEffectType::AddAffinity;
	Chat.Effect.Target = Npc;
	Chat.Effect.Value = 3;

	FDialogueChoice Gossip = MakeChoice(TEXT("问问楼里最近的事"), TEXT("gossip"));
	Gossip.Effect.Type = EDialogueEffectType::AddAffinity;
	Gossip.Effect.Target = Npc;
	Gossip.Effect.Value = 2;

	// 好感够了，Uncle 请喝 kopi（小奖励 +$1）。
	FDialogueChoice Kopi = MakeChoice(TEXT("跟 Uncle 聊到他要请喝 kopi"), TEXT("kopi"));
	Kopi.Condition.Type = EDialogueConditionType::MinAffinity;
	Kopi.Condition.Target = Npc;
	Kopi.Condition.Value = 60;
	Kopi.Effect.Type = EDialogueEffectType::AddMoneyCents;
	Kopi.Effect.Value = 100; // +$1

	// 看了十几年的保安，最有七月的故事。（恐怖方向插入桥段）
	FDialogueChoice Ghost = MakeChoice(TEXT("问 Uncle 这栋楼有没有什么『不干净』的事"), TEXT("ghost"));

	FDialogueTree Tree;
	Tree.TreeId = Npc;
	Tree.RootNodeId = TEXT("root");
	Tree.Nodes = {
		MakeNode(TEXT("root"), Speaker, TEXT("回来啦？今天加班到这么晚，辛苦咯。"),
			{ Chat, Gossip, Kopi, Ghost, MakeEndChoice(TEXT("点头致意")) }),

		MakeNode(TEXT("chat"), Speaker, TEXT("有什么事按门铃找我，叔叔在这看了十几年咯。"),
			{ MakeEndChoice(TEXT("有劳 Uncle")) }),

		MakeNode(TEXT("gossip"), Speaker, TEXT("三楼那家上个月搬走了，听说去 BTO 排到组屋了，啧，年轻人有出息。"),
			{ MakeEndChoice(TEXT("羡慕啊")) }),

		MakeNode(TEXT("kopi"), Speaker, TEXT("走，楼下 kopi 我请！别推辞，叔叔难得碰到聊得来的。"),
			{ MakeEndChoice(TEXT("那就谢谢 Uncle 啦")) }),

		MakeNode(TEXT("ghost"), Speaker, TEXT("……你也是七月才问这个。十几年前 13 楼那间，半夜电梯老停在那层，门开了没人。后来封了。"),
			{ MakeChoice(TEXT("那现在呢？"), TEXT("ghost2")), MakeEndChoice(TEXT("我还是别问了")) }),

		MakeNode(TEXT("ghost2"), Speaker, TEXT("现在？七月你晚上回来，电梯要是自己停在 13 楼，记得——别进去，等下一趟。"),
			{ MakeEndChoice(TEXT("……我记住了")) }),
	};
	return Tree;
}

FDialogueTree SGDialogueContent::BuildColleagueWeiTree()
{
	const FText Speaker = FText::FromString(TEXT("同事 Wei"));
	const FName Npc = TEXT("Wei");

	FDialogueChoice Chat = MakeChoice(TEXT("吐槽两句今天的活"), TEXT("chat"));
	Chat.Effect.Type = EDialogueEffectType::AddAffinity;
	Chat.Effect.Target = Npc;
	Chat.Effect.Value = 3;

	FDialogueChoice Career = MakeChoice(TEXT("问问他怎么涨薪比较快"), TEXT("career"));
	Career.Effect.Type = EDialogueEffectType::AddAffinity;
	Career.Effect.Target = Npc;
	Career.Effect.Value = 2;

	// 好感够了，Wei 帮内推 —— 解锁成就。
	FDialogueChoice Referral = MakeChoice(TEXT("聊到他愿意帮你内推"), TEXT("referral"));
	Referral.Condition.Type = EDialogueConditionType::MinAffinity;
	Referral.Condition.Target = Npc;
	Referral.Condition.Value = 50;
	Referral.Effect.Type = EDialogueEffectType::MarkAchievement;
	Referral.Effect.Target = SGAchievementIds::KnowColleague();

	FDialogueTree Tree;
	Tree.TreeId = Npc;
	Tree.RootNodeId = TEXT("root");
	Tree.Nodes = {
		MakeNode(TEXT("root"), Speaker, TEXT("哟，也来这边吃啊？坐坐坐，今天那个线上 bug 你看了没？"),
			{ Chat, Career, Referral, MakeEndChoice(TEXT("改天聊")) }),

		MakeNode(TEXT("chat"), Speaker, TEXT("PM 又临时加需求，我都麻了。在这行啊，习惯就好。"),
			{ MakeEndChoice(TEXT("同感同感")) }),

		MakeNode(TEXT("career"), Speaker, TEXT("说真的，跳槽涨得比死等升职快多了。骑驴找马，机会到了就跳。"),
			{ MakeEndChoice(TEXT("受教了")) }),

		MakeNode(TEXT("referral"), Speaker, TEXT("我们组在招人，待遇不错。你简历发我，我帮你内推一下！"),
			{ MakeEndChoice(TEXT("太够意思了，谢谢 Wei！")) }),
	};
	return Tree;
}

TArray<FDialogueTree> SGDialogueContent::BuildAllTrees()
{
	return { BuildAhHuaTree(), BuildAhMeiTree(), BuildUncleLimTree(), BuildColleagueWeiTree() };
}
