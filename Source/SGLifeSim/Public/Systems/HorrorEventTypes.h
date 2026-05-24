#pragma once

#include "CoreMinimal.h"
#include "HorrorEventTypes.generated.h"

/**
 * 恐怖事件（深夜/农历七月）。新加坡都市传说 + 中元节禁忌 + 异乡人心理恐怖。
 * 大框架不变，恐怖按桥段插入 —— 见 docs/decisions/2026-05-24-first-person-horror-pivot.md。
 */
UENUM(BlueprintType)
enum class EHorrorEvent : uint8
{
	None                 UMETA(DisplayName = "无事"),
	CorridorLights       UMETA(DisplayName = "走廊感应灯熄灭"),
	ElevatorGhostFloor   UMETA(DisplayName = "电梯停在空楼层"),
	NeighbourEmptyFlat   UMETA(DisplayName = "空屋拖椅声"),
	MrtNoReflection      UMETA(DisplayName = "地铁里没有倒影的人"),
	ChangiHospitalTale   UMETA(DisplayName = "旧樟宜医院的传闻"),
	DeportationNightmare UMETA(DisplayName = "被遣返的噩梦"),
	ZhiQianTaboo         UMETA(DisplayName = "七月冥纸禁忌"),       // 鬼月限定
	KopiWarning          UMETA(DisplayName = "咖啡店的提醒"),       // 鬼月限定
	Pontianak            UMETA(DisplayName = "组屋楼下的白影"),     // 鬼月限定，稀有
	Count                UMETA(Hidden),
};

/** 一个恐怖事件的定义。 */
USTRUCT(BlueprintType)
struct FHorrorEventDef
{
	GENERATED_BODY()

	/** 弹给玩家的阴森文案。 */
	UPROPERTY(BlueprintReadOnly, Category = "Horror")
	FText Title;

	/** 心情变化（恐惧，通常为负）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Horror")
	int32 MoodDelta = 0;

	/** 健康变化（吓到/没睡好，通常为负或 0）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Horror")
	int32 HealthDelta = 0;

	/** 加权随机的基础权重。 */
	UPROPERTY(BlueprintReadOnly, Category = "Horror")
	int32 Weight = 0;

	/** 仅在农历七月（鬼月）可能发生。 */
	UPROPERTY(BlueprintReadOnly, Category = "Horror")
	bool bGhostMonthOnly = false;
};
