#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Systems/DialogueSystem.h"
#include "Systems/SGDialogueTypes.h"
#include "DialogueSubsystem.generated.h"

/** 对话状态变化（开始/选择/结束）时广播，供 UI 刷新。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueChanged);

/**
 * 对话子系统。spec §6.3 + ADR 0005。
 *
 * UE5 GameInstanceSubsystem 薄壳：持一个 FDialogueSystem + 对话树注册表，
 * 把对话的「条件判定」接到 relationship/residency/progress，把「效果应用」
 * 写回 relationship/economy/progress。对话树数据驱动（当前在 C++ 建示例树，
 * 后续可换 DataTable / 资产）。
 */
UCLASS()
class SGLIFESIM_API UDialogueSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** 开始一棵已注册的对话树。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Dialogue")
	bool StartDialogue(FName TreeId);

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Dialogue")
	bool IsDialogueActive() const { return Dialogue.IsActive(); }

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Dialogue")
	FText GetCurrentSpeaker() const;

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Dialogue")
	FText GetCurrentLine() const;

	/** 当前可见（条件满足）的选项文本，顺序与 ChooseOption 的 VisibleIndex 对应。 */
	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Dialogue")
	TArray<FText> GetChoiceTexts() const;

	/** 选第 VisibleIndex 个可见选项：施加效果并推进。成功返回 true。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Dialogue")
	bool ChooseOption(int32 VisibleIndex);

	UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Dialogue")
	FOnDialogueChanged OnDialogueChanged;

	/** 注册一棵对话树（测试 / 内容加载用）。 */
	void RegisterTree(const FDialogueTree& Tree) { Trees.Add(Tree.TreeId, Tree); }

	FDialogueSystem& GetDialogue() { return Dialogue; }

private:
	FDialogueSystem Dialogue;

	UPROPERTY()
	TMap<FName, FDialogueTree> Trees;

	/** 条件求值：读 relationship/residency/progress。 */
	bool EvaluateCondition(const FDialogueCondition& Condition) const;

	/** 效果应用：写 relationship/economy/progress。 */
	void ApplyEffect(const FDialogueEffect& Effect);

	/** 建内置示例对话树。 */
	void BuildSampleTrees();
};
