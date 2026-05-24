#include "Systems/SaveGameSubsystem.h"
#include "Systems/SGSaveGame.h"
#include "Systems/TimeSubsystem.h"
#include "Systems/EconomySubsystem.h"
#include "Systems/ProgressSubsystem.h"
#include "Systems/RelationshipSubsystem.h"
#include "Systems/PlayerStateSubsystem.h"
#include "Systems/ResidencySubsystem.h"
#include "Systems/AssetsSubsystem.h"
#include "Systems/CareerSubsystem.h"
#include "Systems/EndingSubsystem.h"
#include "Kismet/GameplayStatics.h"

const FString USaveGameSubsystem::DefaultSlot = TEXT("SGLifeSim_Slot0");

void USaveGameSubsystem::GatherInto(USGSaveGame& Save) const
{
	const UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	if (const UTimeSubsystem* Time = GI->GetSubsystem<UTimeSubsystem>())
	{
		Save.TimeTotalBlocks = Time->GetTime().GetTotalBlocks();
	}
	if (const UEconomySubsystem* Eco = GI->GetSubsystem<UEconomySubsystem>())
	{
		Save.EconomyBalances = Eco->GetEconomy().GetBalancesSnapshot();
		Save.EconomyTransactions = Eco->GetEconomy().GetTransactions();
	}
	if (const UProgressSubsystem* Prog = GI->GetSubsystem<UProgressSubsystem>())
	{
		Save.Achievements = Prog->GetProgress().GetAchievedSet().Array();
	}
	if (const URelationshipSubsystem* Rel = GI->GetSubsystem<URelationshipSubsystem>())
	{
		Save.Affinities = Rel->GetRelationship().GetAllAffinities();
	}
	if (const UPlayerStateSubsystem* PS = GI->GetSubsystem<UPlayerStateSubsystem>())
	{
		Save.PlayerAttributes = PS->GetStats().GetSnapshot();
	}
	if (const UResidencySubsystem* Res = GI->GetSubsystem<UResidencySubsystem>())
	{
		Save.ResidencyStatus = Res->GetStatus();
		Save.PRRejectionCount = Res->GetPRRejectionCount();
	}
	if (const UAssetsSubsystem* Assets = GI->GetSubsystem<UAssetsSubsystem>())
	{
		Save.HousingTier = Assets->GetHousingTier();
		Save.VehicleTier = Assets->GetVehicleTier();
		Save.InvestmentCents = Assets->GetInvestmentValue();
		const FMortgage& M = Assets->GetAssets().GetMortgage();
		Save.MortgageOutstandingCents = M.OutstandingPrincipalCents;
		Save.MortgageAnnualRatePerMille = M.AnnualRatePerMille;
		Save.MortgageMonthlyPrincipalCents = M.MonthlyPrincipalCents;
	}
	if (const UCareerSubsystem* Career = GI->GetSubsystem<UCareerSubsystem>())
	{
		Save.CareerLevel = Career->GetLevel();
		Save.CareerGrossSalaryCents = Career->GetGrossSalaryCents();
		Save.CareerMonthsInRole = Career->GetMonthsInRole();
	}
	if (const UEndingSubsystem* End = GI->GetSubsystem<UEndingSubsystem>())
	{
		Save.ChosenEnding = End->GetChosenEnding();
	}
}

void USaveGameSubsystem::ApplyFrom(const USGSaveGame& Save)
{
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	if (UTimeSubsystem* Time = GI->GetSubsystem<UTimeSubsystem>())
	{
		Time->GetTime().RestoreTotalBlocks(Save.TimeTotalBlocks);
	}
	if (UEconomySubsystem* Eco = GI->GetSubsystem<UEconomySubsystem>())
	{
		Eco->GetEconomy().RestoreBalances(Save.EconomyBalances);
		Eco->GetEconomy().RestoreTransactions(Save.EconomyTransactions);
	}
	if (UProgressSubsystem* Prog = GI->GetSubsystem<UProgressSubsystem>())
	{
		Prog->GetProgress().RestoreAchieved(Save.Achievements);
	}
	if (URelationshipSubsystem* Rel = GI->GetSubsystem<URelationshipSubsystem>())
	{
		Rel->GetRelationship().RestoreAffinities(Save.Affinities);
	}
	if (UPlayerStateSubsystem* PS = GI->GetSubsystem<UPlayerStateSubsystem>())
	{
		PS->GetStats().RestoreSnapshot(Save.PlayerAttributes);
	}
	if (UResidencySubsystem* Res = GI->GetSubsystem<UResidencySubsystem>())
	{
		Res->GetResidency().RestoreState(Save.ResidencyStatus, Save.PRRejectionCount);
	}
	if (UAssetsSubsystem* Assets = GI->GetSubsystem<UAssetsSubsystem>())
	{
		Assets->GetAssets().RestoreState(Save.HousingTier, Save.VehicleTier, Save.InvestmentCents,
			Save.MortgageOutstandingCents, Save.MortgageAnnualRatePerMille, Save.MortgageMonthlyPrincipalCents);
	}
	if (UCareerSubsystem* Career = GI->GetSubsystem<UCareerSubsystem>())
	{
		Career->GetCareer().RestoreState(Save.CareerLevel, Save.CareerGrossSalaryCents, Save.CareerMonthsInRole);
		Career->SyncSalaryToEconomy(); // 读档后把薪资重新推给 Economy
	}
	if (UEndingSubsystem* End = GI->GetSubsystem<UEndingSubsystem>())
	{
		End->RestoreChosenEnding(Save.ChosenEnding);
	}
}

bool USaveGameSubsystem::SaveToSlot(const FString& SlotName)
{
	USGSaveGame* Save = Cast<USGSaveGame>(
		UGameplayStatics::CreateSaveGameObject(USGSaveGame::StaticClass()));
	if (!Save)
	{
		return false;
	}

	GatherInto(*Save);
	Save->SavedAtUtc = FDateTime::UtcNow();
	return UGameplayStatics::SaveGameToSlot(Save, SlotName, 0);
}

bool USaveGameSubsystem::LoadFromSlot(const FString& SlotName)
{
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		return false;
	}

	USGSaveGame* Save = Cast<USGSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (!Save)
	{
		return false;
	}

	ApplyFrom(*Save);
	return true;
}

bool USaveGameSubsystem::DoesSaveExist(const FString& SlotName) const
{
	return UGameplayStatics::DoesSaveGameExist(SlotName, 0);
}
