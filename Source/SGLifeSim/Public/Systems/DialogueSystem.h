#pragma once

#include "CoreMinimal.h"
#include "Templates/Function.h"
#include "Systems/SGDialogueTypes.h"

/**
 * 对话树运行时。spec §6.3。
 *
 * 纯 C++ 状态机：持一棵树 + 当前节点，按注入的「条件求值器」过滤可选项、
 * 把选中项的效果产出给调用方应用。条件/效果是纯数据，核心不碰具体系统——
 * 可用 lambda 求值器单测，UDialogueSubsystem 才把条件/效果接到真实系统。
 */
class SGLIFESIM_API FDialogueSystem
{
public:
	/** 条件求值器：给定条件返回是否满足。Type==None 由核心直接当满足，不会进这里。 */
	using FConditionEvaluator = TFunctionRef<bool(const FDialogueCondition&)>;

	/** 开始一棵对话树（定位到根节点）。根节点不存在则不激活。 */
	void Start(const FDialogueTree& InTree);

	bool IsActive() const { return bActive; }

	/** 当前节点（未激活返回 nullptr）。 */
	const FDialogueNode* GetCurrentNode() const;

	/** 当前节点中条件满足、可见的选项下标（指向该节点 Choices 的原始下标）。 */
	TArray<int32> GetAvailableChoiceIndices(FConditionEvaluator Evaluator) const;

	/**
	 * 选择某选项（原始下标）。校验激活 + 下标合法 + 条件满足。
	 * 成功：OutEffect = 该选项主效果；跳到 NextNodeId（None/EndDialogue/目标缺失则结束）。
	 * @return 成功返回 true；否则 false 且不改状态。
	 * @note 单效果便利重载。多效果选项请用返回 TArray 的重载，否则只拿到主效果。
	 */
	bool TryChoose(int32 ChoiceIndex, FConditionEvaluator Evaluator, FDialogueEffect& OutEffect);

	/**
	 * 同上，但产出该选项的全部生效效果（主 + 额外，跳过 None/EndDialogue）。
	 * 一个选项可同时回理智 + 加好感等，不必拆成两个选项。
	 * @return 成功返回 true；否则 false 且不改状态。
	 */
	bool TryChoose(int32 ChoiceIndex, FConditionEvaluator Evaluator, TArray<FDialogueEffect>& OutEffects);

	/** 强制结束。 */
	void End() { bActive = false; }

	/**
	 * 静态校验一棵对话树的完整性（内容作者防呆）：
	 * 根节点必须存在；每个选项若指定了 NextNodeId（非空且不是 EndDialogue 效果），
	 * 该目标节点必须存在于树中。合法返回 true，否则 OutError 填首个问题。
	 */
	static bool ValidateTree(const FDialogueTree& InTree, FString& OutError);

private:
	const FDialogueNode* FindNode(FName NodeId) const;
	static bool IsChoiceAvailable(const FDialogueChoice& Choice, FConditionEvaluator Evaluator);

	FDialogueTree Tree;
	FName CurrentNodeId;
	bool bActive = false;
};
