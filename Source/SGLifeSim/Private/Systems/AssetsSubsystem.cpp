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

	UEconomySubsystem* Eco = GetGameInstance() ? GetGameInstance()->GetSubsystem<UEconomySubsystem>() : nullptr;

	const int32 CurrentMonth = TimeSys->GetMonthNumber();
	bool bChanged = false;
	while (CurrentMonth > LastReturnMonth)
	{
		++LastReturnMonth;
		Assets.AccrueInvestmentReturn(MonthlyReturnPerMille);

		// 月供：有按揭就强制扣现金（允许欠债 = 逾期，体现房贷焦虑，不 hard fail）。
		if (Eco && Assets.HasMortgage())
		{
			const int64 Payment = Assets.GetMortgage().PayScheduledMonth();
			Eco->GetEconomy().Charge(ECurrencyAccount::Cash, Payment, TEXT("Mortgage"));
		}
		bChanged = true;
	}
	if (bChanged)
	{
		OnAssetsChanged.Broadcast();
	}
}

bool UAssetsSubsystem::BuyHousingFinanced(EHousingTier Tier)
{
	UEconomySubsystem* Eco = GetGameInstance() ? GetGameInstance()->GetSubsystem<UEconomySubsystem>() : nullptr;
	if (!Eco) { return false; }

	const int64 Price = FAssetsSystem::HousingValuationCents(Tier);
	if (Price <= 0)
	{
		return false; // 只有自购 tier 能按揭
	}

	const int64 DownPayment = Price * (int64)FMath::Clamp(DownPaymentPercent, 0, 100) / 100;
	const int64 Loan = Price - DownPayment;

	// 首付付得起才成交。
	if (!Eco->TryWithdraw(ECurrencyAccount::Cash, DownPayment, TEXT("HousingDownPayment")))
	{
		return false;
	}
	Assets.SetHousingTier(Tier);
	Assets.OpenMortgage(Loan, MortgageAnnualRatePerMille, MortgageTenureMonths);
	OnAssetsChanged.Broadcast();
	return true;
}

bool UAssetsSubsystem::PrepayMortgage()
{
	UEconomySubsystem* Eco = GetGameInstance() ? GetGameInstance()->GetSubsystem<UEconomySubsystem>() : nullptr;
	if (!Eco || !Assets.HasMortgage()) { return false; }

	const int64 Payoff = Assets.GetMortgage().PayoffAmountCents();
	if (!Eco->TryWithdraw(ECurrencyAccount::Cash, Payoff, TEXT("MortgagePrepay")))
	{
		return false;
	}
	Assets.GetMortgage().Clear();
	OnAssetsChanged.Broadcast();
	return true;
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
