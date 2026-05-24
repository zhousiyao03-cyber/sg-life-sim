#pragma once

#include "CoreMinimal.h"
#include "SGDialogueTypes.generated.h"

/** 选项的门控条件类型。spec §6.3。 */
UENUM(BlueprintType)
enum class EDialogueConditionType : uint8
{
	None            UMETA(DisplayName = "无条件"),
	MinAffinity     UMETA(DisplayName = "好感≥"),
	MaxAffinity     UMETA(DisplayName = "好感≤"),
	MinResidency    UMETA(DisplayName = "身份≥"),   // Value = EResidencyStatus 的 int 值
	HasAchievement  UMETA(DisplayName = "已达成成就"),
};

/** 选中选项后施加的效果类型。 */
UENUM(BlueprintType)
enum class EDialogueEffectType : uint8
{
	None            UMETA(DisplayName = "无"),
	AddAffinity     UMETA(DisplayName = "加好感"),
	AddMoneyCents   UMETA(DisplayName = "加钱(分)"),   // 可负 = 花钱
	MarkAchievement UMETA(DisplayName = "解锁成就"),
	EndDialogue     UMETA(DisplayName = "结束对话"),
};

/** 一个门控条件。Target/Value 含义随 Type 而定。 */
USTRUCT(BlueprintType)
struct FDialogueCondition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	EDialogueConditionType Type = EDialogueConditionType::None;

	/** NPC id（好感）/ 成就 id；其余类型忽略。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FName Target;

	/** 阈值（好感分 / 身份枚举 int）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	int32 Value = 0;
};

/** 一个效果。 */
USTRUCT(BlueprintType)
struct FDialogueEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	EDialogueEffectType Type = EDialogueEffectType::None;

	/** NPC id（好感）/ 成就 id。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FName Target;

	/** 数值（好感增量 / 钱分数）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	int64 Value = 0;
};

/** 一个对话选项。 */
USTRUCT(BlueprintType)
struct FDialogueChoice
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText Text;

	/** 选后跳转到的节点；None = 选后结束对话。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FName NextNodeId;

	/** 此选项的可见门控。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FDialogueCondition Condition;

	/** 选中时施加的效果。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FDialogueEffect Effect;
};

/** 一个对话节点。 */
USTRUCT(BlueprintType)
struct FDialogueNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FName NodeId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText Speaker;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText Line;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TArray<FDialogueChoice> Choices;
};

/** 一棵对话树。 */
USTRUCT(BlueprintType)
struct FDialogueTree
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FName TreeId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FName RootNodeId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TArray<FDialogueNode> Nodes;
};
