#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactables/InteractableInterface.h"
#include "SGInteractableNPC.generated.h"

class USkeletalMeshComponent;
class USphereComponent;

/**
 * 可交互 NPC。spec §7.2。实现 IInteractableInterface，玩家走近按交互键触发对话。
 *
 * 原型阶段对话内容是占位文本；Task 9 起接入 UMG 对话框 / 后续接对话系统。
 */
UCLASS()
class SGLIFESIM_API ASGInteractableNPC : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	ASGInteractableNPC();

	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() const override;

	/** 「Speaker：Line」格式的整句，供玩家 HUD 对话气泡显示。 */
	FText GetDialogueDisplayText() const;

	/**
	 * 代码生成 NPC 时的初始化（设 NpcId / 说话人 / 占位台词 / 骨骼网格）。
	 * 供 USGWorldPopulatorSubsystem 在关卡加载时填充世界用，免手摆 .umap。
	 */
	void ConfigureNpc(FName InNpcId, const FText& InSpeakerName, const FString& InDialogueLine,
		USkeletalMesh* InMesh = nullptr);

	/** 关系系统的稳定 key（如 AhHua / Auntie）。供 URelationshipSubsystem 记好感。 */
	FName GetNpcId() const { return NpcId; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SGLifeSim|NPC")
	TObjectPtr<USkeletalMeshComponent> NpcMesh;

	/** 交互检测范围（玩家进入此球体即可交互）。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SGLifeSim|NPC")
	TObjectPtr<USphereComponent> InteractionRange;

	/** 关系系统用的稳定标识（英文 key，区别于可本地化的 SpeakerName）。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SGLifeSim|NPC")
	FName NpcId = TEXT("AhHua");

	/** 对话框显示的说话人名字。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SGLifeSim|NPC")
	FText SpeakerName;

	/** NPC 说的一句话（原型占位）。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SGLifeSim|NPC", meta = (MultiLine = true))
	FText DialogueLine;
};
