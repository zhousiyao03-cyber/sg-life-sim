#include "Systems/DialogueSystem.h"

void FDialogueSystem::Start(const FDialogueTree& InTree)
{
	Tree = InTree;
	CurrentNodeId = Tree.RootNodeId;
	bActive = (FindNode(CurrentNodeId) != nullptr);
}

const FDialogueNode* FDialogueSystem::FindNode(FName NodeId) const
{
	for (const FDialogueNode& Node : Tree.Nodes)
	{
		if (Node.NodeId == NodeId)
		{
			return &Node;
		}
	}
	return nullptr;
}

const FDialogueNode* FDialogueSystem::GetCurrentNode() const
{
	return bActive ? FindNode(CurrentNodeId) : nullptr;
}

bool FDialogueSystem::IsChoiceAvailable(const FDialogueChoice& Choice, FConditionEvaluator Evaluator)
{
	if (Choice.Condition.Type == EDialogueConditionType::None)
	{
		return true;
	}
	return Evaluator(Choice.Condition);
}

TArray<int32> FDialogueSystem::GetAvailableChoiceIndices(FConditionEvaluator Evaluator) const
{
	TArray<int32> Out;
	const FDialogueNode* Node = GetCurrentNode();
	if (!Node)
	{
		return Out;
	}
	for (int32 i = 0; i < Node->Choices.Num(); ++i)
	{
		if (IsChoiceAvailable(Node->Choices[i], Evaluator))
		{
			Out.Add(i);
		}
	}
	return Out;
}

bool FDialogueSystem::TryChoose(int32 ChoiceIndex, FConditionEvaluator Evaluator, FDialogueEffect& OutEffect)
{
	const FDialogueNode* Node = GetCurrentNode();
	if (!Node || !Node->Choices.IsValidIndex(ChoiceIndex))
	{
		return false;
	}

	const FDialogueChoice& Choice = Node->Choices[ChoiceIndex];
	if (!IsChoiceAvailable(Choice, Evaluator))
	{
		return false;  // 条件不满足，拒绝
	}

	OutEffect = Choice.Effect;

	// 跳转：EndDialogue / 无下一节点 / 目标节点缺失 → 结束。
	if (Choice.Effect.Type == EDialogueEffectType::EndDialogue
		|| Choice.NextNodeId.IsNone()
		|| FindNode(Choice.NextNodeId) == nullptr)
	{
		bActive = false;
	}
	else
	{
		CurrentNodeId = Choice.NextNodeId;
	}
	return true;
}
