#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Systems/PlayerStats.h"
#include "Systems/PlayerStatsTypes.h"
#include "Systems/TimeBlock.h"
#include "PlayerStateSubsystem.generated.h"

/** 某属性变化时广播。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnAttributeChanged, EPlayerAttribute, Attribute, int32, NewValue);

/**
 * 主角状态子系统。spec §6.4 + ADR 0005。
 *
 * UE5 GameInstanceSubsystem 薄壳，内部委托给纯 C++ 的 FPlayerStats。
 * 暴露 6 个属性给 Blueprint，订阅 TimeSubsystem 的跨天事件做能量每日恢复。
 */
UCLASS()
class SGLIFESIM_API UPlayerStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// UGameInstanceSubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Player")
	int32 GetAttribute(EPlayerAttribute Attr) const;

	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Player")
	void SetAttribute(EPlayerAttribute Attr, int32 Value);

	/** 增减属性（活动消耗 / 奖励），结果 clamp。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Player")
	void ModifyAttribute(EPlayerAttribute Attr, int32 Delta);

	UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Player")
	FOnAttributeChanged OnAttributeChanged;

	/** 直接访问内部纯 C++ 状态（供存档 / 其他 C++ 系统用）。 */
	FPlayerStats& GetStats() { return Stats; }
	const FPlayerStats& GetStats() const { return Stats; }

private:
	FPlayerStats Stats;

	/** 上一次记录的天号，用于检测跨天 → 能量恢复。 */
	int32 LastDayNumber = 1;

	UFUNCTION()
	void HandleTimeAdvanced(ETimeBlock NewBlock, int32 DayNumber);

	void NotifyAttribute(EPlayerAttribute Attr);
};
