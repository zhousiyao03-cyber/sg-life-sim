#pragma once

#include "CoreMinimal.h"
#include "WeaponTypes.generated.h"

/** 武器种类（C 块 GTA 枪械）。None = 徒手（只能出拳）。 */
UENUM(BlueprintType)
enum class EWeaponKind : uint8
{
	None    UMETA(DisplayName = "徒手"),
	Pistol  UMETA(DisplayName = "手枪"),
	Rifle   UMETA(DisplayName = "步枪"),
};

/**
 * 武器静态参数。纯数据，开火规则在 FWeaponSystem 里。
 */
struct FWeaponDef
{
	int32 Damage = 0;        // 每发伤害
	int32 MagSize = 0;       // 弹匣容量
	int32 WantedPerShot = 0; // 每次开火涨多少通缉值
	float Range = 0.f;       // 射程（cm）
};
