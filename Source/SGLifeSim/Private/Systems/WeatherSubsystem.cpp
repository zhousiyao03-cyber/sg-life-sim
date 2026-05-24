#include "Systems/WeatherSubsystem.h"
#include "Systems/WeatherSystem.h"
#include "Systems/TimeSubsystem.h"

#include "Engine/GameInstance.h"

void UWeatherSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 用确定性种子起步（晴），订阅时间推进跨天换天气。
	WeatherStream.Initialize(20260524);
	CurrentWeather = EWeather::Clear;
	LastSeenDay = 1;

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UTimeSubsystem* Time = GI->GetSubsystem<UTimeSubsystem>())
		{
			Time->OnTimeAdvanced.AddUniqueDynamic(this, &UWeatherSubsystem::HandleTimeAdvanced);
			LastSeenDay = Time->GetDayNumber();
		}
	}
}

float UWeatherSubsystem::GetWeatherFogDensity() const
{
	return FWeatherSystem::FogDensity(CurrentWeather);
}

bool UWeatherSubsystem::IsRaining() const
{
	return FWeatherSystem::IsRaining(CurrentWeather);
}

void UWeatherSubsystem::SetWeather(EWeather NewWeather)
{
	if (NewWeather == CurrentWeather) { return; }
	CurrentWeather = NewWeather;
	OnWeatherChanged.Broadcast(CurrentWeather);
}

void UWeatherSubsystem::RollNewWeather()
{
	SetWeather(FWeatherSystem::PickNext(WeatherStream));
}

void UWeatherSubsystem::HandleTimeAdvanced(ETimeBlock NewBlock, int32 DayNumber)
{
	// 跨天才换天气（一天内天气稳定）。
	if (DayNumber != LastSeenDay)
	{
		LastSeenDay = DayNumber;
		RollNewWeather();
	}
}
