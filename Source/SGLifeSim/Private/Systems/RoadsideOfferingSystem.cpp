#include "Systems/RoadsideOfferingSystem.h"

FRoadsideOfferingOutcome FRoadsideOfferingSystem::Resolve(ERoadsideOfferingChoice Choice, FRandomStream& Stream)
{
	FRoadsideOfferingOutcome Out;

	switch (Choice)
	{
	case ERoadsideOfferingChoice::DetourAround:
		Out.Message = FText::FromString(TEXT("你绕了一大圈，多走了好几条街。脚酸，但你一步都没踩到那些金纸——这就够了。"));
		Out.SanityDelta = DetourSanityGain;
		Out.EnergyDelta = -DetourEnergyCost;
		Out.bSomethingHappened = false;
		break;

	case ERoadsideOfferingChoice::PayRespects:
		Out.Message = FText::FromString(TEXT("你停下，双手合十，心里念了句『打扰了，借过』。风停了一瞬。你绕过祭品，脚步轻了许多。"));
		Out.SanityDelta = PayRespectsSanityGain;
		Out.EnergyDelta = -PayRespectsEnergyCost;
		Out.bSomethingHappened = false;
		break;

	case ERoadsideOfferingChoice::StepOver:
	{
		// 犯禁忌，赌一把。Stream.RandRange(0,99) < 概率 = 出事。
		const bool bBad = Stream.RandRange(0, 99) < StepOverBadPercent;
		Out.EnergyDelta = -StepOverEnergyCost;
		if (bBad)
		{
			Out.Message = FText::FromString(TEXT("你一脚跨过那堆金纸。身后『啪』地一声——回头看，烧了一半的纸钱散了一地，可刚才明明压着石头。你加快了脚步。"));
			Out.SanityDelta = -StepOverBadSanityCost;
			Out.bSomethingHappened = true;
		}
		else
		{
			Out.Message = FText::FromString(TEXT("你跨了过去，什么都没发生。可走出去老远，你才发现自己一直屏着呼吸。"));
			Out.SanityDelta = -StepOverOkSanityCost;
			Out.bSomethingHappened = false;
		}
		break;
	}

	default:
		break;
	}

	return Out;
}
