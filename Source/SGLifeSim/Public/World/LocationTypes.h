#pragma once

#include "CoreMinimal.h"
#include "Systems/ActivityTypes.h"
#include "LocationTypes.generated.h"

/**
 * 游戏里的地点（开放城市枢纽，2026-05-24）。每个地点对应一个室内关卡，
 * 在城市地图上有一栋可进入的建筑。见 docs/superpowers/specs/2026-05-24-open-city-hub-design.md。
 */
UENUM(BlueprintType)
enum class ELocation : uint8
{
	None      UMETA(Hidden),
	Rental    UMETA(DisplayName = "出租屋"),
	Hawker    UMETA(DisplayName = "食阁"),
	Office    UMETA(DisplayName = "公司"),
	Corridor  UMETA(DisplayName = "组屋楼道"),
	MRT       UMETA(DisplayName = "地铁站"),
	Mall      UMETA(DisplayName = "商场"),

	Count     UMETA(Hidden),
};

/**
 * 一个地点的定义（纯数据，供 FLocationRegistry 查、可单测）。
 * 城市建筑摆哪、进哪个关卡、室内能做什么，全从这张表读 —— 单一事实源。
 */
USTRUCT(BlueprintType)
struct FLocationDef
{
	GENERATED_BODY()

	/** 室内关卡名（OpenLevel 用 / 当前关卡匹配用）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Location")
	FName LevelName;

	/** 显示名（HUD「[E] 进入 X」、小地图标签）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Location")
	FText DisplayName;

	/** 在城市地图上的世界坐标（建筑入口摆这里；小地图按比例描点）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Location")
	FVector CityLocation = FVector::ZeroVector;

	/** 这个地点室内可做的活动（活动菜单据此过滤）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Location")
	TArray<EActivityType> Activities;
};
