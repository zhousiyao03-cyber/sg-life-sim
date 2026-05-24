#include "Systems/DialogueSubsystem.h"
#include "Systems/SGDialogueContent.h"
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
	// 对话内容集中在 SGDialogueContent（数据驱动，可单测 ValidateTree）。
	for (const FDialogueTree& Tree : SGDialogueContent::BuildAllTrees())
	{
		RegisterTree(Tree);
	}
}
