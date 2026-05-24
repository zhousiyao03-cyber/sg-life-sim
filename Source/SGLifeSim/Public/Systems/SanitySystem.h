#pragma once

#include "CoreMinimal.h"
#include "Systems/SanityTypes.h"

/**
 * 理智纯逻辑核心。零 UE 子系统依赖，可单测。
 * 理智越低，深夜越容易、越凶地出恐怖事件（恐惧螺旋）。
 */
class SGLIFESIM_API FSanitySystem
{
public:
	static constexpr int32 MinSanity = 0;
	static constexpr int32 MaxSanity = 100;

	static int32 Clamp(int32 Value) { return FMath::Clamp(Value, MinSanity, MaxSanity); }

	/** 理智值 → 状态档。 */
	static ESanityState GetState(int32 Sanity)
	{
		if (Sanity >= 70) { return ESanityState::Calm; }
		if (Sanity >= 40) { return ESanityState::Uneasy; }
		if (Sanity >= 15) { return ESanityState::Disturbed; }
		return ESanityState::Breaking;
	}

	/**
	 * 低理智给恐怖事件「加注」的额外权重：从「无事」权重里扣掉，越低理智越容易出事。
	 * 平静 0 / 不安 10 / 失常 25 / 濒临崩溃 45。
	 */
	static int32 ExtraDreadWeight(int32 Sanity)
	{
		switch (GetState(Sanity))
		{
		case ESanityState::Calm:      return 0;
		case ESanityState::Uneasy:    return 10;
		case ESanityState::Disturbed: return 25;
		case ESanityState::Breaking:  return 45;
		default:                      return 0;
		}
	}
};
