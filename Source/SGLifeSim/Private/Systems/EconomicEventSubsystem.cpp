#include "Systems/EconomicEventSubsystem.h"
#include "Systems/EconomySubsystem.h"
#include "Systems/EconomyTypes.h"
#include "Systems/AssetsSubsystem.h"
#include "Systems/TimeSubsystem.h"

void UEconomicEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency(UTimeSubsystem::StaticClass());
	Collection.InitializeDependency(UEconomySubsystem::StaticClass());
	Collection.InitializeDependency(UAssetsSubsystem::StaticClass());

	// 默认用一个随时间变化的种子，让每局体验不同；测试可 SetSeed 覆盖。
	Stream.Initialize((int32)(FDateTime::Now().GetTicks() & 0x7fffffff));

	if (UTimeSubsystem* TimeSys = GetGameInstance()->GetSubsystem<UTimeSubsystem>())
	{
		TimeSys->OnTimeAdvanced.AddDynamic(this, &UEconomicEventSubsystem::HandleTimeAdvanced);
		LastRolledMonth = TimeSys->GetMonthNumber();
	}
}

void UEconomicEventSubsystem::Deinitialize()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UTimeSubsystem* TimeSys = GI->GetSubsystem<UTimeSubsystem>())
		{
			TimeSys->OnTimeAdvanced.RemoveDynamic(this, &UEconomicEventSubsystem::HandleTimeAdvanced);
		}
	}
	Super::Deinitialize();
}

void UEconomicEventSubsystem::HandleTimeAdvanced(ETimeBlock NewBlock, int32 DayNumber)
{
	UTimeSubsystem* TimeSys = GetGameInstance() ? GetGameInstance()->GetSubsystem<UTimeSubsystem>() : nullptr;
	if (!TimeSys) { return; }

	const int32 CurrentMonth = TimeSys->GetMonthNumber();
	while (CurrentMonth > LastRolledMonth)
	{
		++LastRolledMonth;
		if (bEnabled)
		{
			ApplyEvent(FEconomicEventSystem::PickEvent(Stream));
		}
	}
}

bool UEconomicEventSubsystem::ApplyEvent(EEconomicEvent Event)
{
	LastEvent = Event;
	if (Event == EEconomicEvent::None)
	{
		return false;
	}

	UGameInstance* GI = GetGameInstance();
	const FEconomicEventDef Def = FEconomicEventSystem::GetEventDef(Event);

	switch (Def.EffectType)
	{
	case EEventEffectType::InvestmentReturnPerMille:
		if (UAssetsSubsystem* Assets = GI ? GI->GetSubsystem<UAssetsSubsystem>() : nullptr)
		{
			Assets->GetAssets().AccrueInvestmentReturn(Def.Magnitude);
		}
		break;

	case EEventEffectType::CashDeltaCents:
		if (UEconomySubsystem* Eco = GI ? GI->GetSubsystem<UEconomySubsystem>() : nullptr)
		{
			if (Def.Magnitude >= 0)
			{
				Eco->Deposit(ECurrencyAccount::Cash, Def.Magnitude, TEXT("Event"));
			}
			else
			{
				Eco->GetEconomy().Charge(ECurrencyAccount::Cash, -Def.Magnitude, TEXT("Event"));
			}
		}
		break;

	case EEventEffectType::CashBonusSalaryMonthsX10:
		if (UEconomySubsystem* Eco = GI ? GI->GetSubsystem<UEconomySubsystem>() : nullptr)
		{
			// Magnitude = 月薪份数 ×10（15 → 1.5 个月）。
			const int64 Bonus = Eco->GetMonthlyGrossSalary() * (int64)Def.Magnitude / 10;
			Eco->Deposit(ECurrencyAccount::Cash, Bonus, TEXT("Bonus"));
		}
		break;

	default:
		break;
	}

	OnEconomicEvent.Broadcast(Def.Title);
	return true;
}
