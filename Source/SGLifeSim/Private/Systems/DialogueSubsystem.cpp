#include "Systems/DialogueSubsystem.h"
#include "Systems/RelationshipSubsystem.h"
#include "Systems/ResidencySubsystem.h"
#include "Systems/ProgressSubsystem.h"
#include "Systems/EconomySubsystem.h"

void UDialogueSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	BuildSampleTrees();
}

bool UDialogueSubsystem::EvaluateCondition(const FDialogueCondition& Condition) const
{
	const UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return false;
	}

	switch (Condition.Type)
	{
	case EDialogueConditionType::None:
		return true;

	case EDialogueConditionType::MinAffinity:
		if (const URelationshipSubsystem* Rel = GI->GetSubsystem<URelationshipSubsystem>())
		{
			return Rel->GetAffinity(Condition.Target) >= Condition.Value;
		}
		return false;

	case EDialogueConditionType::MaxAffinity:
		if (const URelationshipSubsystem* Rel = GI->GetSubsystem<URelationshipSubsystem>())
		{
			return Rel->GetAffinity(Condition.Target) <= Condition.Value;
		}
		return false;

	case EDialogueConditionType::MinResidency:
		if (const UResidencySubsystem* Res = GI->GetSubsystem<UResidencySubsystem>())
		{
			return (int32)Res->GetStatus() >= Condition.Value;
		}
		return false;

	case EDialogueConditionType::HasAchievement:
		if (const UProgressSubsystem* Prog = GI->GetSubsystem<UProgressSubsystem>())
		{
			return Prog->HasAchieved(Condition.Target);
		}
		return false;

	default:
		return false;
	}
}

void UDialogueSubsystem::ApplyEffect(const FDialogueEffect& Effect)
{
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	switch (Effect.Type)
	{
	case EDialogueEffectType::AddAffinity:
		if (URelationshipSubsystem* Rel = GI->GetSubsystem<URelationshipSubsystem>())
		{
			Rel->AddAffinity(Effect.Target, (int32)Effect.Value);
		}
		break;

	case EDialogueEffectType::AddMoneyCents:
		if (UEconomySubsystem* Eco = GI->GetSubsystem<UEconomySubsystem>())
		{
			if (Effect.Value >= 0)
			{
				Eco->Deposit(ECurrencyAccount::Cash, Effect.Value, TEXT("Dialogue"));
			}
			else
			{
				Eco->GetEconomy().Charge(ECurrencyAccount::Cash, -Effect.Value, TEXT("Dialogue"));
			}
		}
		break;

	case EDialogueEffectType::MarkAchievement:
		if (UProgressSubsystem* Prog = GI->GetSubsystem<UProgressSubsystem>())
		{
			Prog->MarkAchieved(Effect.Target);
		}
		break;

	default:
		break;  // None / EndDialogue：无副作用
	}
}

bool UDialogueSubsystem::StartDialogue(FName TreeId)
{
	const FDialogueTree* Tree = Trees.Find(TreeId);
	if (!Tree)
	{
		return false;
	}
	Dialogue.Start(*Tree);
	OnDialogueChanged.Broadcast();
	return Dialogue.IsActive();
}

FText UDialogueSubsystem::GetCurrentSpeaker() const
{
	const FDialogueNode* Node = Dialogue.GetCurrentNode();
	return Node ? Node->Speaker : FText::GetEmpty();
}

FText UDialogueSubsystem::GetCurrentLine() const
{
	const FDialogueNode* Node = Dialogue.GetCurrentNode();
	return Node ? Node->Line : FText::GetEmpty();
}

TArray<FText> UDialogueSubsystem::GetChoiceTexts() const
{
	TArray<FText> Out;
	const FDialogueNode* Node = Dialogue.GetCurrentNode();
	if (!Node)
	{
		return Out;
	}
	auto Eval = [this](const FDialogueCondition& C) { return EvaluateCondition(C); };
	for (int32 Index : Dialogue.GetAvailableChoiceIndices(Eval))
	{
		Out.Add(Node->Choices[Index].Text);
	}
	return Out;
}

bool UDialogueSubsystem::ChooseOption(int32 VisibleIndex)
{
	auto Eval = [this](const FDialogueCondition& C) { return EvaluateCondition(C); };
	const TArray<int32> Available = Dialogue.GetAvailableChoiceIndices(Eval);
	if (!Available.IsValidIndex(VisibleIndex))
	{
		return false;
	}

	FDialogueEffect Effect;
	if (!Dialogue.TryChoose(Available[VisibleIndex], Eval, Effect))
	{
		return false;
	}
	ApplyEffect(Effect);
	OnDialogueChanged.Broadcast();
	return true;
}

void UDialogueSubsystem::BuildSampleTrees()
{
	// 邻居 AhHua 的示例树：闲聊加好感、送礼大幅加好感、高好感解锁交心分支。
	const FText AhHua = FText::FromString(TEXT("邻居 Ah Hua"));

	FDialogueChoice Chat;
	Chat.Text = FText::FromString(TEXT("随便聊两句"));
	Chat.NextNodeId = TEXT("chat");
	Chat.Effect.Type = EDialogueEffectType::AddAffinity;
	Chat.Effect.Target = TEXT("AhHua");
	Chat.Effect.Value = 3;

	FDialogueChoice Gift;
	Gift.Text = FText::FromString(TEXT("送他一盒咖啡乌"));
	Gift.NextNodeId = TEXT("gift");
	Gift.Effect.Type = EDialogueEffectType::AddAffinity;
	Gift.Effect.Target = TEXT("AhHua");
	Gift.Effect.Value = 10;

	FDialogueChoice Heart;
	Heart.Text = FText::FromString(TEXT("聊点掏心窝的"));
	Heart.NextNodeId = TEXT("heart");
	Heart.Condition.Type = EDialogueConditionType::MinAffinity;
	Heart.Condition.Target = TEXT("AhHua");
	Heart.Condition.Value = 50;  // 朋友档才解锁

	FDialogueNode Root;
	Root.NodeId = TEXT("root");
	Root.Speaker = AhHua;
	Root.Line = FText::FromString(TEXT("哟，又见面啦。住得还习惯吗？"));
	Root.Choices = { Chat, Gift, Heart };

	auto MakeBye = []()
	{
		FDialogueChoice Bye;
		Bye.Text = FText::FromString(TEXT("先走了"));
		Bye.Effect.Type = EDialogueEffectType::EndDialogue;
		return Bye;
	};

	FDialogueNode Chat2;
	Chat2.NodeId = TEXT("chat");
	Chat2.Speaker = AhHua;
	Chat2.Line = FText::FromString(TEXT("这边组屋楼下那家鸡饭，便宜又好吃。"));
	Chat2.Choices = { MakeBye() };

	FDialogueNode Gift2;
	Gift2.NodeId = TEXT("gift");
	Gift2.Speaker = AhHua;
	Gift2.Line = FText::FromString(TEXT("哎哟，这么客气！下次来我家喝茶。"));
	Gift2.Choices = { MakeBye() };

	FDialogueNode Heart2;
	Heart2.NodeId = TEXT("heart");
	Heart2.Speaker = AhHua;
	Heart2.Line = FText::FromString(TEXT("说真的，刚来的时候我也熬了好几年才安顿下来。"));
	Heart2.Choices = { MakeBye() };

	FDialogueTree Tree;
	Tree.TreeId = TEXT("AhHua");
	Tree.RootNodeId = TEXT("root");
	Tree.Nodes = { Root, Chat2, Gift2, Heart2 };
	RegisterTree(Tree);
}
