#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "World/LocationTypes.h"
#include "LocationManagerSubsystem.generated.h"

/**
 * 地点进出枢纽总控（开放城市枢纽，2026-05-24）。GameInstanceSubsystem 跨关卡存活，
 * 因为「城市 → 室内 → 回城市」要横跨两次 OpenLevel。
 *
 * EnterLocation：记下玩家在城市的坐标 / 朝向（回程用），OpenLevel 进室内关卡。
 * ReturnToCity：OpenLevel 回城市枢纽，并标记「待传送回门口」——城市加载后由玩家
 * BeginPlay 查 ConsumePendingReturn 取坐标完成传送（OpenLevel 默认重生 PlayerStart，
 * 这里把玩家拉回离开时的建筑门口）。
 *
 * 见 docs/superpowers/specs/2026-05-24-open-city-hub-design.md。
 */
UCLASS()
class SGLIFESIM_API USGLocationManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * 从城市进入一个地点的室内关卡。记下当前城市坐标（回程传送用），OpenLevel 过去。
	 * @return 成功返回 true；地点无效返回 false。
	 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Location")
	bool EnterLocation(ELocation Location);

	/** 从室内离开，回城市枢纽（并安排把玩家传送回离开时的建筑门口）。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Location")
	void ReturnToCity();

	/** 当前所在地点（None=在城市枢纽 / 未知）。 */
	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Location")
	ELocation GetCurrentLocation() const { return CurrentLocation; }

	/**
	 * 城市加载后，玩家取一次「待传送回城市坐标」。
	 * @param OutLocation 待传送到的世界坐标
	 * @param OutRotation 待传送的朝向
	 * @return 有待处理的回程传送返回 true（取后清除，只生效一次）；否则 false（正常出生）。
	 */
	bool ConsumePendingReturn(FVector& OutLocation, FRotator& OutRotation);

	/** 记录当前城市坐标（EnterLocation 内部用；独立出来便于单测，不依赖玩家 pawn）。 */
	void RememberCityTransform(const FVector& Location, const FRotator& Rotation);

private:
	/** 当前所在地点。 */
	ELocation CurrentLocation = ELocation::None;

	/** 离开城市时的坐标 / 朝向（回程传送回这里）。 */
	FVector CityReturnLocation = FVector::ZeroVector;
	FRotator CityReturnRotation = FRotator::ZeroRotator;

	/** 是否有待玩家消费的回程传送。 */
	bool bPendingReturn = false;
};
