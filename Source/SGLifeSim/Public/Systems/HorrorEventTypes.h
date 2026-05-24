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
	MallAfterHours       UMETA(DisplayName = "打烊后的商场"),
	ZhiQianTaboo         UMETA(DisplayName = "七月冥纸禁忌"),       // 鬼月限定
	KopiWarning          UMETA(DisplayName = "咖啡店的提醒"),       // 鬼月限定
	Pontianak            UMETA(DisplayName = "组屋楼下的白影"),     // 鬼月限定，稀有

	// —— 低理智幻觉（理智「失常」档才会出现，分不清真假）——
	HallucCorridorFigure UMETA(DisplayName = "走廊尽头的人影"),     // 低理智限定
	HallucMirrorLag      UMETA(DisplayName = "镜中慢半拍"),         // 低理智限定
	HallucDeportation    UMETA(DisplayName = "醒着的遣返"),         // 低理智限定
	HallucWallFace       UMETA(DisplayName = "墙上的脸"),           // 低理智限定

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

	/** 理智消耗（恐惧，正值=扣多少理智）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Horror")
	int32 SanityCost = 0;

	/** 加权随机的基础权重。 */
	UPROPERTY(BlueprintReadOnly, Category = "Horror")
	int32 Weight = 0;

	/** 仅在农历七月（鬼月）可能发生。 */
	UPROPERTY(BlueprintReadOnly, Category = "Horror")
	bool bGhostMonthOnly = false;

	/** 仅在理智低（失常档及以下）时出现的幻觉 —— 分不清真假。 */
	UPROPERTY(BlueprintReadOnly, Category = "Horror")
	bool bLowSanityOnly = false;
};
