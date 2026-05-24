#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SGCityPopulatorSubsystem.generated.h"

/** 一栋装饰楼的布局（纯数据，可单测城市生成）。 */
struct FDecorBuildingSpec
{
	FVector Location = FVector::ZeroVector;
	FVector Scale = FVector::OneVector;
};

/**
 * 城市枢纽填充（开放城市枢纽，2026-05-24）。WorldSubsystem。
 *
 * L_City BeginPlay 时铺城市：按 FLocationRegistry 在各地点城市坐标 spawn 可进入的
 * ASGBuildingEntrance；再批量 spawn 一片长方体装饰楼（不可进、填充城市感）；spawn 地面。
 * 全代码 spawn、幂等。占位盒子，资产到位后换皮（逻辑不动）。
 *
 * 见 docs/superpowers/specs/2026-05-24-open-city-hub-design.md。
 */
UCLASS()
class SGLIFESIM_API USGCityPopulatorSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	/**
	 * 生成装饰楼布局（纯函数，可单测）：在城市范围内格点铺楼，跳过太靠近任何
	 * 入口建筑坐标的格子（给可进建筑让位）。确定性（同输入同输出）。
	 * @param EntranceLocations 各可进建筑的城市坐标（这些位置附近不放装饰楼）
	 * @param HalfExtent 城市半径（厘米）
	 * @param Spacing 楼间距（厘米）
	 * @param ClearRadius 入口周围多大范围内不放装饰楼
	 */
	static TArray<FDecorBuildingSpec> BuildDecorLayout(
		const TArray<FVector>& EntranceLocations,
		float HalfExtent, float Spacing, float ClearRadius);

private:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
};
