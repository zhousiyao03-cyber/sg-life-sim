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

	/** 出门：从室内关卡回到城市枢纽（走 LocationManager::ReturnToCity，传送回原楼门口）。 */
	UFUNCTION()
	void OnExitToCity();

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

	/** 买车（第8块消费）：花钱在城市生成一辆可驾驶车，停玩家身旁。 */
	UFUNCTION()
	void OnBuyCarClicked();

	/** 下馆子（第8块消费）：花钱回心情。 */
	UFUNCTION()
	void OnDineOutClicked();

	/** 鬼月深夜：进入夜归抉择（展开三个选项，藏起其余菜单项）。 */
	UFUNCTION()
	void OnNightCommuteClicked();

	/** 夜归抉择——等下一趟（安全慢）。 */
	UFUNCTION()
	void OnCommuteWaitClicked();

	/** 夜归抉择——赶紧进去（省事但赌）。 */
	UFUNCTION()
	void OnCommuteStepInClicked();

	/** 夜归抉择——走楼梯（最稳最累）。 */
	UFUNCTION()
	void OnCommuteStairsClicked();

	UFUNCTION()
	void OnCloseClicked();

private:
	/** 移除菜单、恢复游戏输入，然后跳转到目标关卡（Level 为空则只关闭）。 */
	void TravelTo(FName LevelName);

	/** 在菜单里显示一行操作反馈（如「已存档 ✓」）。 */
	void SetStatus(const FString& Message);

	/**
	 * 按当前是否处于「夜归抉择」态刷新各按钮可见性：
	 * 普通态显示日常菜单项（夜归入口仅鬼月深夜可见）；抉择态只露三个选项。
	 */
	void RefreshButtonVisibility();

	/** 当前是否可触发夜归抉择（查 UNightCommuteSubsystem::IsAvailable）。 */
	bool IsNightCommuteAvailable() const;

	/** 做出夜归抉择、关菜单回游戏（结算气泡由 PlayerCharacter 的委托弹）。 */
	void ResolveNightCommute(uint8 Choice);

	/** 是否正处于夜归抉择子界面（决定刷新时露哪组按钮）。 */
	bool bInNightCommute = false;

	/** 出门回城市（仅在室内关卡显示；在城市枢纽里靠走门口按 E 进楼，不用此按钮）。 */
	UPROPERTY(Transient)
	TObjectPtr<UButton> ExitToCityButton;

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

	/** 买车（消费）。 */
	UPROPERTY(Transient)
	TObjectPtr<UButton> BuyCarButton;

	/** 下馆子（消费，回心情）。 */
	UPROPERTY(Transient)
	TObjectPtr<UButton> DineOutButton;

	/** 夜归抉择入口（仅鬼月深夜可见）。 */
	UPROPERTY(Transient)
	TObjectPtr<UButton> NightCommuteButton;

	/** 夜归抉择三选项（仅在抉择态可见）。 */
	UPROPERTY(Transient)
	TObjectPtr<UButton> CommuteWaitButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> CommuteStepInButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> CommuteStairsButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> CloseButton;

	/** 活动菜单（懒创建）。 */
	UPROPERTY(Transient)
	TObjectPtr<USGActivityMenuWidget> ActivityMenu;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatusLabel;
};
