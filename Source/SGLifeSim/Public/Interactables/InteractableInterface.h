#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 可交互对象接口。spec §6.3 关系系统的入口 —— NPC / 物件实现它即可被玩家交互。
 *
 * 用 BlueprintNativeEvent：C++ 提供默认实现，Blueprint 子类可覆写。
 */
class SGLIFESIM_API IInteractableInterface
{
	GENERATED_BODY()

public:
	/** 玩家按下交互键时调用。Interactor 是发起交互的 Actor（通常是主角）。 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SGLifeSim|Interaction")
	void OnInteract(AActor* Interactor);

	/** 玩家走近时显示的提示文本（如 "[E] 对话"）。 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SGLifeSim|Interaction")
	FText GetInteractionPrompt() const;
};
