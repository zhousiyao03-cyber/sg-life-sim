#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SGLocationMenuWidget.generated.h"

class UButton;

/**
 * 地点切换菜单（纯 C++ UMG）。spec 原 plan 的 W_LocationMenu。
 *
 * M 键打开：居中面板列出可去的地点按钮，点一个就 OpenLevel 过去。
 * 打开时切到 UI 输入模式 + 显示鼠标，关闭/前往后切回游戏输入。
 * 控件树全部在 RebuildWidget() 里构造，无需 BP widget 资产。
 */
UCLASS()
class SGLIFESIM_API USGLocationMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 加到视口 + 切 UI 输入模式 + 显示鼠标。 */
	void OpenMenu();

	/** 从视口移除 + 切回游戏输入模式。 */
	void CloseMenu();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UFUNCTION()
	void OnGoRental();

	UFUNCTION()
	void OnGoHawker();

	UFUNCTION()
	void OnCloseClicked();

private:
	/** 移除菜单、恢复游戏输入，然后跳转到目标关卡（Level 为空则只关闭）。 */
	void TravelTo(FName LevelName);

	UPROPERTY(Transient)
	TObjectPtr<UButton> RentalButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> HawkerButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> CloseButton;
};
