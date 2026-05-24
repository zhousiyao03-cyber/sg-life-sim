#include "Systems/NightCommuteSystem.h"

FNightCommuteOutcome FNightCommuteSystem::Resolve(ENightCommuteChoice Choice, FRandomStream& Stream)
{
	FNightCommuteOutcome Out;

	switch (Choice)
	{
	case ENightCommuteChoice::WaitForNext:
		Out.Message = FText::FromString(TEXT("你后退一步，让门合上。等了很久，下一趟电梯空荡荡地来了。你照 Uncle 说的做了。"));
		Out.SanityDelta = WaitSanityGain;
		Out.EnergyDelta = -WaitEnergyCost;
		Out.bSomethingHappened = false;
		break;

	case ENightCommuteChoice::TakeStairs:
		Out.Message = FText::FromString(TEXT("你转身走楼梯。十三层，腿快断了，但你一次都没敢回头——平安到家。"));
		Out.SanityDelta = 0;
		Out.EnergyDelta = -StairsEnergyCost;
		Out.bSomethingHappened = false;
		break;

	case ENightCommuteChoice::StepIn:
	{
		// 犯禁忌，赌一把。Stream.RandRange(0,99) < 概率 = 出事。
		const bool bBad = Stream.RandRange(0, 99) < StepInBadPercent;
		Out.EnergyDelta = -StepInEnergyCost;
		if (bBad)
		{
			Out.Message = FText::FromString(TEXT("电梯门一关，灯灭了一瞬。再亮起时，你分明听见身后有人，跟你一起进来了——可镜里只有你。"));
			Out.SanityDelta = -StepInBadSanityCost;
			Out.bSomethingHappened = true;
		}
		else
		{
			Out.Message = FText::FromString(TEXT("电梯安安静静把你送到家。什么都没发生。可你后背的冷汗，过了好久才干。"));
			Out.SanityDelta = -StepInOkSanityCost;
			Out.bSomethingHappened = false;
		}
		break;
	}

	default:
		break;
	}

	return Out;
}
