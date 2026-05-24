#pragma once

#include "CoreMinimal.h"
#include "Systems/HorrorEventTypes.h"
#include "HorrorSceneTypes.generated.h"

/**
 * 恐怖场景（Plan 24）。把重磅恐怖事件从「文案 + 数值」升级为「真实场景 + 锁视角演出」：
 * 触发时 OpenLevel 进专属关卡走一段脚本化演出，演完送回原关卡并结算。
 *
 * 第一条链:Elevator（电梯空楼层）。验证整条技术链后再扩到其他事件。
 * 见 docs/superpowers/specs/2026-05-24-horror-scene-sequence-design.md。
 */
UENUM(BlueprintType)
enum class EHorrorScene : uint8
{
	None      UMETA(Hidden),
	Elevator  UMETA(DisplayName = "电梯空楼层"),

	Count     UMETA(Hidden),
};

/** 一个恐怖场景的定义（纯数据，供 Registry 查、可单测）。 */
USTRUCT(BlueprintType)
struct FHorrorSceneDef
{
	GENERATED_BODY()

	/** 演出所在的关卡名（OpenLevel 用）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Horror")
	FName LevelName;

	/** 演完结算扣多少理智（正值）。场景演出比纯文案更痛。 */
	UPROPERTY(BlueprintReadOnly, Category = "Horror")
	int32 SanityCost = 0;

	/** 记进图鉴的对应都市传说条目。 */
	UPROPERTY(BlueprintReadOnly, Category = "Horror")
	EHorrorEvent CodexEntry = EHorrorEvent::None;

	/** 送回原关卡后弹给玩家的事后文案（松一口气 / 后怕）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Horror")
	FText AftermathText;
};
