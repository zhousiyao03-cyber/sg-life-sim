#pragma once

#include "CoreMinimal.h"

/**
 * 玩家生命值纯逻辑核心（B 块 GTA：血量/死亡/重生）。零 UE 子系统依赖，可单测。
 *
 * 血量 0~100。受击扣血、归零即死亡；重生不在此处（属世界/经济副作用，由 Subsystem 壳做）。
 * 这里只管「数值怎么变、何时算死」这一纯规则。
 */
class SGLIFESIM_API FPlayerVitalsSystem
{
public:
	static constexpr int32 MinHealth = 0;
	static constexpr int32 MaxHealth = 100;

	static int32 Clamp(int32 Value) { return FMath::Clamp(Value, MinHealth, MaxHealth); }

	/** 扣血（返回扣后血量，已 clamp）。负 Damage 无意义，按 0 处理。 */
	static int32 ApplyDamage(int32 Health, int32 Damage)
	{
		if (Damage < 0) { Damage = 0; }
		return Clamp(Health - Damage);
	}

	/** 回血（返回回后血量，已 clamp）。负 Amount 按 0 处理。 */
	static int32 Heal(int32 Health, int32 Amount)
	{
		if (Amount < 0) { Amount = 0; }
		return Clamp(Health + Amount);
	}

	/** 是否已死亡（血量见底）。 */
	static bool IsDead(int32 Health) { return Health <= MinHealth; }
};
