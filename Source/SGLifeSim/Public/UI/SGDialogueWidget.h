#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SGDialogueWidget.generated.h"

class UTextBlock;
class UButton;
class UVerticalBox;
class UDialogueSubsystem;

/**
 * 对话界面（纯 C++ UMG）。Plan 6「对话 UI」。
 *
 * 把 UDialogueSubsystem 的运行时状态显示出来：底部对话面板 = 说话人 + 台词 +
 * 一列可点选项按钮。选项按当前条件门控（由 Subsystem 求值），点击映射到
 * UDialogueSubsystem::ChooseOption(VisibleIndex)。订阅 OnDialogueChanged 自动刷新，
 * 对话结束（IsDialogueActive 变 false）时自动关闭并交还输入。
 *
 * 控件树全部在 RebuildWidget() 用 WidgetTree 构造，无需 BP widget 资产 ——
 * 与 USGHudWidget / USGLocationMenuWidget 一致（spec §10.3）。
 *
 * 选项按钮预建固定 MaxChoices 个，刷新时只改文本/可见性，结构稳定。
 */
UCLASS()
class SGLIFESIM_API USGDialogueWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 选项按钮上限（足够覆盖示例树；超出的选项不显示）。 */
	static constexpr int32 MaxChoices = 6;

	/** 开始展示某棵对话树：注册到 viewport、切输入模式、订阅刷新。成功返回 true。 */
	bool OpenForTree(FName TreeId);

	/** 关闭对话界面：取消订阅、移出 viewport、交还输入给游戏。 */
	void Close();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	/** 从 UDialogueSubsystem 拉当前说话人/台词/可见选项，刷新控件。 */
	void Refresh();

	/** 拿 GameInstance 上的对话子系统（可能为空）。 */
	UDialogueSubsystem* GetDialogueSubsystem() const;

	/** OnDialogueChanged 回调：刷新；若对话已结束则关闭。 */
	UFUNCTION()
	void HandleDialogueChanged();

	/** 选第 N 个可见选项。 */
	void Choose(int32 VisibleIndex);

	// 固定 6 个选项按钮各一个点击回调（UButton::OnClicked 不带参数，用固定下标映射）。
	UFUNCTION() void OnChoice0();
	UFUNCTION() void OnChoice1();
	UFUNCTION() void OnChoice2();
	UFUNCTION() void OnChoice3();
	UFUNCTION() void OnChoice4();
	UFUNCTION() void OnChoice5();

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SpeakerText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> LineText;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> ChoicesBox;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UButton>> ChoiceButtons;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> ChoiceLabels;

	/** 是否已订阅子系统委托，避免重复绑定/解绑。 */
	bool bSubscribed = false;
};
