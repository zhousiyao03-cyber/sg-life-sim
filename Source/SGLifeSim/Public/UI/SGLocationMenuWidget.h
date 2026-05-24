#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SGLocationMenuWidget.generated.h"

class UButton;
class UTextBlock;
class USGActivityMenuWidget;

/**
 * 游戏菜单（纯 C++ UMG）。spec 原 plan 的 W_LocationMenu。
 *
 * M 键打开：居中面板列出可去的地点 + 存档 / 读档按钮。
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
	void OnSaveClicked();

	UFUNCTION()
	void OnLoadClicked();

	/** 按揭买组屋（首付从现金扣，余额开房贷）。 */
	UFUNCTION()
	void OnBuyHdbFinancedClicked();

	/** 提前结清房贷。 */
	UFUNCTION()
	void OnPrepayMortgageClicked();

	/** 申请升职。 */
	UFUNCTION()
	void OnPromoteClicked();

	/** 跳槽（+35%）。 */
	UFUNCTION()
	void OnJobHopClicked();

	/** 打开活动菜单（在当前地点做点事）。 */
	UFUNCTION()
	void OnDoActivitiesClicked();

	UFUNCTION()
	void OnCloseClicked();

private:
	/** 移除菜单、恢复游戏输入，然后跳转到目标关卡（Level 为空则只关闭）。 */
	void TravelTo(FName LevelName);

	/** 在菜单里显示一行操作反馈（如「已存档 ✓」）。 */
	void SetStatus(const FString& Message);

	UPROPERTY(Transient)
	TObjectPtr<UButton> RentalButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> HawkerButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> SaveButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> LoadButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> BuyHdbButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> PrepayButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> PromoteButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> JobHopButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> ActivitiesButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> CloseButton;

	/** 活动菜单（懒创建）。 */
	UPROPERTY(Transient)
	TObjectPtr<USGActivityMenuWidget> ActivityMenu;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatusLabel;
};
