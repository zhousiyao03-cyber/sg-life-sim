#include "Systems/EndingSubsystem.h"
#include "Systems/EndingEvaluator.h"
#include "Systems/ResidencySubsystem.h"
#include "Systems/AssetsSubsystem.h"
#include "Systems/RelationshipSubsystem.h"
#include "Systems/EconomySubsystem.h"

int64 UEndingSubsystem::ComputeTotalNetWorth() const
{
	const UGameInstance* GI = GetGameInstance();
	if (!GI) { return 0; }

	int64 Total = 0;
	if (const UEconomySubsystem* Eco = GI->GetSubsystem<UEconomySubsystem>())
	{
		Total += Eco->GetNetWorth();
	}
	if (const UAssetsSubsystem* Assets = GI->GetSubsystem<UAssetsSubsystem>())
	{
		Total += Assets->GetAssetNetWorthContribution();
	}
	return Total;
}

int32 UEndingSubsystem::ComputeMaxAffinity() const
{
	const UGameInstance* GI = GetGameInstance();
	if (!GI) { return 0; }

	int32 MaxAff = 0;
	if (const URelationshipSubsystem* Rel = GI->GetSubsystem<URelationshipSubsystem>())
	{
		for (const TPair<FName, int32>& Pair : Rel->GetRelationship().GetAllAffinities())
		{
			MaxAff = FMath::Max(MaxAff, Pair.Value);
		}
	}
	return MaxAff;
}

EEnding UEndingSubsystem::GetCurrentLeaning() const
{
	const UGameInstance* GI = GetGameInstance();
	if (!GI) { return EEnding::None; }

	EResidencyStatus Status = EResidencyStatus::WorkPermit_EP;
	int32 PRRejections = 0;
	if (const UResidencySubsystem* Res = GI->GetSubsystem<UResidencySubsystem>())
	{
		Status = Res->GetStatus();
		PRRejections = Res->GetPRRejectionCount();
	}

	bool bOwnsHome = false;
	if (const UAssetsSubsystem* Assets = GI->GetSubsystem<UAssetsSubsystem>())
	{
		bOwnsHome = Assets->OwnsHome();
	}

	return FEndingEvaluator::EvaluateLeaning(
		Status, bOwnsHome, ComputeMaxAffinity(), ComputeTotalNetWorth(), PRRejections);
}

void UEndingSubsystem::ChooseEnding(EEnding Ending)
{
	ChosenEnding = Ending;
	OnEndingChosen.Broadcast(Ending);
}
