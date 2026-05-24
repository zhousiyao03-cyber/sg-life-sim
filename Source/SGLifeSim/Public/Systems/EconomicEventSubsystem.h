#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Math/RandomStream.h"
#include "Systems/EconomicEventSystem.h"
#include "Systems/EconomicEventTypes.h"
#include "Systems/TimeBlock.h"
#include "EconomicEventSubsystem.generated.h"

/** 触发非平静经济事件时广播事件标题，供 HUD 弹 toast。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEconomicEvent, FText, Title);

/**
 * 经济事件子系统。spec §6.2 + ADR 0005。
 *
 * 持一条 FRandomStream，订阅 TimeSubsystem 月初抽一个事件（多数月平静），
 * 把效果应用到 Economy（现金/年终奖）与 Assets（投资涨跌），非平静则广播给 UI。
 * 抽取逻辑在纯 C++ 的 FEconomicEventSystem，可注入种子做确定性测试。
 */
UCLASS()
class SGLIFESIM_API UEconomicEventSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 设随机种子（测试 / 新周目用），让事件序列可复现。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Events")
	void SetSeed(int32 Seed) { Stream.Initialize(Seed); }

	/** 开关月度随机事件。关掉后跨月不抽事件（供需要确定性经济的测试用）。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Events")
	void SetEventsEnabled(bool bInEnabled) { bEnabled = bInEnabled; }

	/** 应用一个指定事件的效果（月度抽取与脚本/测试共用）。非平静返回 true 并广播。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Events")
	bool ApplyEvent(EEconomicEvent Event);

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Events")
	EEconomicEvent GetLastEvent() const { return LastEvent; }

	UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Events")
	FOnEconomicEvent OnEconomicEvent;

private:
	FRandomStream Stream;

	UPROPERTY()
	EEconomicEvent LastEvent = EEconomicEvent::None;

	/** 上次抽取的月号，检测跨月。 */
	int32 LastRolledMonth = 1;

	/** 是否启用月度随机事件（默认开；测试可关以保证确定性）。 */
	bool bEnabled = true;

	UFUNCTION()
	void HandleTimeAdvanced(ETimeBlock NewBlock, int32 DayNumber);
};
