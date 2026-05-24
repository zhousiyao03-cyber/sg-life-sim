#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Math/RandomStream.h"

#include "Systems/WeatherSystem.h"
#include "Systems/WeatherTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * 天气纯核心（H 块 GTA）：加权随机挑天气可复现，雾浓度随恶劣天气递增。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWeatherSystemTest,
	"SGLifeSim.World.WeatherSystem",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWeatherSystemTest::RunTest(const FString& Parameters)
{
	using W = FWeatherSystem;

	// 雾浓度排序：晴 < 多云 < 雨 < 雾。
	TestTrue(TEXT("clear < cloudy"), W::FogDensity(EWeather::Clear) < W::FogDensity(EWeather::Cloudy));
	TestTrue(TEXT("cloudy < rain"), W::FogDensity(EWeather::Cloudy) < W::FogDensity(EWeather::Rain));
	TestTrue(TEXT("rain < fog"), W::FogDensity(EWeather::Rain) < W::FogDensity(EWeather::Fog));

	// 只有雨算下雨。
	TestTrue(TEXT("rain is raining"), W::IsRaining(EWeather::Rain));
	TestFalse(TEXT("clear not raining"), W::IsRaining(EWeather::Clear));

	// 加权随机：多次抽样四种天气都应出现，且晴的出现频率高于雾（权重 40 vs 5）。
	{
		FRandomStream S(777);
		int32 Counts[4] = { 0, 0, 0, 0 };
		for (int32 i = 0; i < 2000; ++i)
		{
			Counts[(int32)W::PickNext(S)]++;
		}
		TestTrue(TEXT("clear appears"),  Counts[(int32)EWeather::Clear]  > 0);
		TestTrue(TEXT("cloudy appears"), Counts[(int32)EWeather::Cloudy] > 0);
		TestTrue(TEXT("rain appears"),   Counts[(int32)EWeather::Rain]   > 0);
		TestTrue(TEXT("fog appears"),    Counts[(int32)EWeather::Fog]    > 0);
		TestTrue(TEXT("clear more common than fog"),
			Counts[(int32)EWeather::Clear] > Counts[(int32)EWeather::Fog]);
	}

	// 同种子可复现。
	{
		FRandomStream A(123), B(123);
		bool bSame = true;
		for (int32 i = 0; i < 100; ++i)
		{
			if (W::PickNext(A) != W::PickNext(B)) { bSame = false; break; }
		}
		TestTrue(TEXT("same seed -> same weather sequence"), bSame);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
