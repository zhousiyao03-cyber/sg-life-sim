#pragma once

#include "CoreMinimal.h"
#include "Systems/WeaponTypes.h"

/**
 * 枪械纯逻辑核心（C 块 GTA）。零 UE 子系统依赖，可单测。
 * 管「武器参数表、能否开火、开火后弹匣怎么减、换弹补多少」这些纯规则；
 * 射线命中与造成伤害的世界交互在 Subsystem / 玩家角色侧做。
 */
class SGLIFESIM_API FWeaponSystem
{
public:
	/** 取武器静态参数。None 返回全 0。 */
	static FWeaponDef GetDef(EWeaponKind Kind)
	{
		FWeaponDef D;
		switch (Kind)
		{
		case EWeaponKind::Pistol:
			D.Damage = 34; D.MagSize = 12; D.WantedPerShot = 30; D.Range = 8000.f; break;
		case EWeaponKind::Rifle:
			D.Damage = 55; D.MagSize = 30; D.WantedPerShot = 45; D.Range = 15000.f; break;
		case EWeaponKind::None:
		default:
			break;
		}
		return D;
	}

	/** 是否持有真正的枪（能开火）。 */
	static bool CanFireKind(EWeaponKind Kind) { return Kind != EWeaponKind::None; }

	/** 当前能否开火：有枪且弹匣里还有子弹。 */
	static bool CanFire(EWeaponKind Kind, int32 AmmoInMag)
	{
		return CanFireKind(Kind) && AmmoInMag > 0;
	}

	/** 开火消耗一发，返回剩余弹药（调用前应先 CanFire 判定）。 */
	static int32 ConsumeRound(int32 AmmoInMag)
	{
		return FMath::Max(0, AmmoInMag - 1);
	}

	/** 换弹：把弹匣补满到容量（无限备弹的简化模型），返回换弹后弹药。 */
	static int32 Reload(EWeaponKind Kind)
	{
		return GetDef(Kind).MagSize;
	}
};
