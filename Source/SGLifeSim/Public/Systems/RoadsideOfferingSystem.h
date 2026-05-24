#pragma once

#include "CoreMinimal.h"
#include "Systems/RoadsideOfferingTypes.h"
#include "Math/RandomStream.h"

/**
 * 鬼月路边祭品抉择纯逻辑核心（Plan 26）。零 UE 子系统依赖，可单测。
 *
 * 路中间的祭品/金纸前做选择：绕开（安全慢）/ 跨过去（省事但赌）/ 拜一拜（最稳还回理智）。
 * 「跨过去」按概率结算——犯了禁忌不一定当场出事，赌输了重扣理智。注入 FRandomStream → 可复现。
 *
 * 与 FNightCommuteSystem 同构，但「拜一拜」是主动回理智（敬畏化解恐惧），区别于纯绕路。
 */
class SGLIFESIM_API FRoadsideOfferingSystem
{
public:
	/** 「跨过去」犯禁忌出事的概率（百分比）。比电梯略低——祭品更多靠运气，但一样在赌。 */
	static constexpr int32 StepOverBadPercent = 50;

	/** 各选项的精力代价。 */
	static constexpr int32 DetourEnergyCost   = 10;  // 绕一大圈
	static constexpr int32 StepOverEnergyCost = 2;   // 最省事
	static constexpr int32 PayRespectsEnergyCost = 5; // 停一下，不太累

	/** 赌输（跨过去招事）扣的理智 —— 重，因为是自找的、还是犯了忌讳。 */
	static constexpr int32 StepOverBadSanityCost = 20;

	/** 赌赢（跨过去没事）也心虚，轻扣。 */
	static constexpr int32 StepOverOkSanityCost = 5;

	/** 绕开走，守规矩，理智几乎无损（略安心）。 */
	static constexpr int32 DetourSanityGain = 2;

	/** 拜一拜，敬畏化解恐惧 —— 回理智最多（本地人的智慧）。 */
	static constexpr int32 PayRespectsSanityGain = 8;

	/**
	 * 结算一次抉择。Stream 仅在「跨过去」时消费（决定有没有出事）。
	 * @return 结果（文案 + 理智/精力变化 + 是否出事）。
	 */
	static FRoadsideOfferingOutcome Resolve(ERoadsideOfferingChoice Choice, FRandomStream& Stream);
};
