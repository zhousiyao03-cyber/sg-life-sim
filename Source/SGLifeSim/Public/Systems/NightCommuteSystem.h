#pragma once

#include "CoreMinimal.h"
#include "Systems/NightCommuteTypes.h"
#include "Math/RandomStream.h"

/**
 * 鬼月夜归抉择纯逻辑核心（Plan 23）。零 UE 子系统依赖，可单测。
 *
 * 玩家在 13 楼空楼层电梯前做选择：等下一趟（安全慢）/ 赶紧进去（省事但赌）/ 走楼梯（最稳最累）。
 * 「赶紧进去」按概率结算——犯了禁忌不一定当场出事，但赌输了重扣理智。注入 FRandomStream → 可复现。
 */
class SGLIFESIM_API FNightCommuteSystem
{
public:
	/** 「赶紧进去」出事的概率（百分比）。守规矩的人都知道这是在赌。 */
	static constexpr int32 StepInBadPercent = 55;

	/** 各选项的精力代价（守规矩越费劲）。 */
	static constexpr int32 WaitEnergyCost   = 8;   // 站着干等
	static constexpr int32 StepInEnergyCost = 2;   // 最省事
	static constexpr int32 StairsEnergyCost = 20;  // 13 层楼，腿废

	/** 赌输（进去出事）扣的理智 —— 重，因为是自找的。 */
	static constexpr int32 StepInBadSanityCost = 22;

	/** 赌赢（进去没事）也吓出一身汗，轻扣。 */
	static constexpr int32 StepInOkSanityCost = 4;

	/** 等下一趟，乖乖守规矩，理智几乎无损（甚至因「做对了」略安心）。 */
	static constexpr int32 WaitSanityGain = 2;

	/**
	 * 结算一次抉择。Stream 仅在「赶紧进去」时消费（决定有没有出事）。
	 * @return 结果（文案 + 理智/精力变化 + 是否出事）。
	 */
	static FNightCommuteOutcome Resolve(ENightCommuteChoice Choice, FRandomStream& Stream);
};
