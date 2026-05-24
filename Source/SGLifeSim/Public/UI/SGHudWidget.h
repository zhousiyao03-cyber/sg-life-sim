#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SGHudWidget.generated.h"

class UTextBlock;
class UCanvasPanel;

/**
 * 原型 HUD（纯 C++ UMG）。spec §10.3：核心在 C++，不依赖 BP 控件树。
 *
 * 取代之前的 AddOnScreenDebugMessage 屏幕文本 —— 那套在实机可见但不进截图。
 * 控件树全部在 RebuildWidget() 里用 WidgetTree 构造，无需 BP widget 资产，
 * 由 ASGPlayerCharacter 在 BeginPlay 里 CreateWidget + AddToViewport 并每帧刷新。
 */
UCLASS()
class SGLIFESIM_API USGHudWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 顶部状态行：「Day X · 周几 · 时间块 + 操作提示」。 */
	void SetStatusText(const FText& InText);

	/** 靠近可交互对象时的提示（如「[E] 对话」）；传空则隐藏。 */
	void SetPromptText(const FText& InText);

	/** 对话气泡（「Speaker：Line」）；传空则隐藏。 */
	void SetDialogueText(const FText& InText);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PromptText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> DialogueText;
};
