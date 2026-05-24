#include "Systems/AssetsSubsystem.h"
#include "Systems/EconomySubsystem.h"
#include "Systems/TimeSubsystem.h"

void UAssetsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency(UTimeSubsystem::StaticClass());
	Collection.InitializeDependency(UEconomySubsystem::StaticClass());
	if (UTimeSubsystem* TimeSys = GetGameInstance()->GetSubsystem<UTimeSubsystem>())
	{
		TimeSys->OnTimeAdvanced.AddDynamic(this, &UAssetsSubsystem::HandleTimeAdvanced);
		LastReturnMonth = TimeSys->GetMonthNumber();
	}
}

void UAssetsSubsystem::Deinitialize()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UTimeSubsystem* TimeSys = GI->GetSubsystem<UTimeSubsystem>())
		{
			TimeSys->OnTimeAdvanced.RemoveDynamic(this, &UAssetsSubsystem::HandleTimeAdvanced);
		}
	}
	Super::Deinitialize();
}

void UAssetsSubsystem::HandleTimeAdvanced(ETimeBlock NewBlock, int32 DayNumber)
{
	UTimeSubsystem* TimeSys = GetGameInstance() ? GetGameInstance()->GetSubsystem<UTimeSubsystem>() : nullptr;
	if (!TimeSys) { return; }

	const int32 CurrentMonth = TimeSys->GetMonthNumber();
	bool bAccrued = false;
	while (CurrentMonth > LastReturnMonth)
	{
		++LastReturnMonth;
		Assets.AccrueInvestmentReturn(MonthlyReturnPerMille);
		bAccrued = true;
	}
	if (bAccrued)
	{
		OnAssetsChanged.Broadcast();
	}
}

bool UAssetsSubsystem::BuyHousing(EHousingTier Tier)
{
	UEconomySubsystem* Eco = GetGameInstance() ? GetGameInstance()->GetSubsystem<UEconomySubsystem>() : nullptr;
	if (!Eco) { return false; }

	const int64 Cost = FAssetsSystem::HousingValuationCents(Tier);
	// 免费 tier（无 / 租房）直接设；自购 tier 需扣款。
	if (Cost > 0 && !Eco->TryWithdraw(ECurrencyAccount::Cash, Cost, TEXT("BuyHousing")))
	{
		return false;
	}
	Assets.SetHousingTier(Tier);
	OnAssetsChanged.Broadcast();
	return true;
}

bool UAssetsSubsystem::BuyVehicle(EVehicleTier Tier)
{
	UEconomySubsystem* Eco = GetGameInstance() ? GetGameInstance()->GetSubsystem<UEconomySubsystem>() : nullptr;
	if (!Eco) { return false; }

	const int64 Cost = FAssetsSystem::VehicleValuationCents(Tier);
	if (Cost > 0 && !Eco->TryWithdraw(ECurrencyAccount::Cash, Cost, TEXT("BuyVehicle")))
	{
		return false;
	}
	Assets.SetVehicleTier(Tier);
	OnAssetsChanged.Broadcast();
	return true;
}

bool UAssetsSubsystem::Invest(int64 Cents)
{
	UEconomySubsystem* Eco = GetGameInstance() ? GetGameInstance()->GetSubsystem<UEconomySubsystem>() : nullptr;
	if (!Eco || Cents <= 0) { return false; }

	if (!Eco->TryWithdraw(ECurrencyAccount::Cash, Cents, TEXT("Invest")))
	{
		return false;
	}
	Assets.AddInvestment(Cents);
	OnAssetsChanged.Broadcast();
	return true;
}

int64 UAssetsSubsystem::Divest(int64 Cents)
{
	UEconomySubsystem* Eco = GetGameInstance() ? GetGameInstance()->GetSubsystem<UEconomySubsystem>() : nullptr;
	if (!Eco) { return 0; }

	const int64 Got = Assets.WithdrawInvestment(Cents);
	if (Got > 0)
	{
		Eco->Deposit(ECurrencyAccount::Cash, Got, TEXT("Divest"));
		OnAssetsChanged.Broadcast();
	}
	return Got;
}
