#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Systems/ActivityTypes.h"
#include "SGActivityMenuWidget.generated.h"

class UTextBlock;
class UButton;
class UVerticalBox;
class UActivitySubsystem;

/**
 * 活动菜单（纯 C++ UMG）。Plan 10「活动循环」。
 *
 * 列出当前地点可做的活动（出租屋：睡觉/学习/接私活/健身；食阁：吃饭/听八卦），
 * 点一个 → UActivitySubsystem::PerformActivity（消耗时间块、改属性/钱）→ 刷新。
 * 由 USGLocationMenuWidget 的「做点事…」按钮打开。预建固定 MaxActivities 个按钮，
 * 刷新时改文本/可见/可用，结构稳定（与对话/菜单 widget 一致）。
 */
UCLASS()
class SGLIFESIM_API USGActivityMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	static constexpr int32 MaxActivities = (int32)EActivityType::Count;

	/** 加视口 + UI 输入模式 + 显示鼠标 + 刷新。 */
	void OpenMenu();

	/** 移出视口 + 交还游戏输入。 */
	void CloseMenu();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void Refresh();
	UActivitySubsystem* GetActivitySubsystem() const;
	void Choose(int32 VisibleIndex);

	UFUNCTION() void OnActivity0();
	UFUNCTION() void OnActivity1();
	UFUNCTION() void OnActivity2();
	UFUNCTION() void OnActivity3();
	UFUNCTION() void OnActivity4();
	UFUNCTION() void OnActivity5();

	UFUNCTION() void OnCloseClicked();

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UButton>> ActivityButtons;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> ActivityLabels;

	UPROPERTY(Transient)
	TObjectPtr<UButton> CloseButton;

	/** 当前可见按钮 → 活动类型映射（Refresh 时重建）。 */
	TArray<EActivityType> VisibleActivities;
};
