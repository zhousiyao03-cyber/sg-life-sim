#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Systems/HorrorEventTypes.h"
#include "Systems/HorrorSceneTypes.h"
#include "Systems/TimeBlock.h"
#include "Math/RandomStream.h"
#include "HorrorEventSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHorrorEvent, FText, Title);

/** 带事件类型的版本（图鉴等需要知道「是哪一个」而非仅文案）。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHorrorEventTyped, EHorrorEvent, Event);

/**
 * 恐怖事件子系统。FHorrorEventSystem 的 UE 薄壳。
 *
 * 每天入夜（LateNight）掷一次：农历七月（鬼月）出事概率更高、且解锁鬼月限定事件。
 * 命中则按定义扣心情/健康，并广播 OnHorrorEvent（玩家 HUD 弹阴森文案）。
 * 注入种子可复现；SetEnabled(false) 供确定性测试关闭随机。
 */
UCLASS()
class SGLIFESIM_API UHorrorEventSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 恐怖事件发生时广播阴森文案。 */
	UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Horror")
	FOnHorrorEvent OnHorrorEvent;

	/** 恐怖事件发生时广播事件类型（图鉴收集用）。 */
	UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Horror")
	FOnHorrorEventTyped OnHorrorEventTyped;

	/** 当前是否农历七月（鬼月）。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Horror")
	bool IsGhostMonth() const;

	/** 施加一个恐怖事件（扣属性 + 广播）。None 返回 false。供测试与直接触发用。 */
	bool ApplyEvent(EHorrorEvent Event);

	/** 关随机（确定性测试用）。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Horror")
	void SetHorrorEnabled(bool bInEnabled) { bEnabled = bInEnabled; }

	/** 设种子（复现用）。 */
	void SetSeed(int32 Seed) { Stream.Initialize(Seed); }

	EHorrorEvent GetLastEvent() const { return LastEvent; }

	/** 某恐怖事件对应的专属场景演出（None=无专属场景，走文案气泡）。加场景只改这映射。 */
	static EHorrorScene SceneForEvent(EHorrorEvent Event);

private:
	UFUNCTION()
	void HandleTimeAdvanced(ETimeBlock NewBlock, int32 DayNumber);

	FRandomStream Stream;
	EHorrorEvent LastEvent = EHorrorEvent::None;
	bool bEnabled = true;

	/** 已掷骰子的最近一天，保证每天入夜只掷一次。 */
	int32 LastRolledDay = -1;
};
