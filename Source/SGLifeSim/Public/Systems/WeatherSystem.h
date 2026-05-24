#pragma once

#include "CoreMinimal.h"
#include "Math/RandomStream.h"
#include "Systems/WeatherTypes.h"

/**
 * 天气纯逻辑核心（H 块 GTA）。零 UE 子系统依赖，可单测。
 * 管「下一个天气怎么挑」（加权随机，注入 RandomStream 可复现）+「天气→雾浓度」映射。
 * 新加坡气候：晴/多云为主，雨频繁（午后雷阵雨），雾偶见。
 */
class SGLIFESIM_API FWeatherSystem
{
public:
	/** 各天气的相对权重（新加坡：晴多云为主、雨不少、雾稀少）。 */
	static int32 Weight(EWeather W)
	{
		switch (W)
		{
		case EWeather::Clear:  return 40;
		case EWeather::Cloudy: return 30;
		case EWeather::Rain:   return 25;
		case EWeather::Fog:    return 5;
		default:               return 0;
		}
	}

	/** 加权随机挑下一个天气（注入 Stream 可复现）。 */
	static EWeather PickNext(FRandomStream& Stream)
	{
		const EWeather All[] = { EWeather::Clear, EWeather::Cloudy, EWeather::Rain, EWeather::Fog };
		int32 Total = 0;
		for (EWeather W : All) { Total += Weight(W); }
		if (Total <= 0) { return EWeather::Clear; }

		int32 Roll = Stream.RandRange(0, Total - 1);
		for (EWeather W : All)
		{
			Roll -= Weight(W);
			if (Roll < 0) { return W; }
		}
		return EWeather::Clear;
	}

	/** 天气 → ExponentialHeightFog 密度（雾最浓，雨次之，晴最清）。 */
	static float FogDensity(EWeather W)
	{
		switch (W)
		{
		case EWeather::Clear:  return 0.02f;
		case EWeather::Cloudy: return 0.05f;
		case EWeather::Rain:   return 0.12f;
		case EWeather::Fog:    return 0.30f;
		default:               return 0.02f;
		}
	}

	/** 是否在下雨（决定要不要播降雨粒子/声音）。 */
	static bool IsRaining(EWeather W) { return W == EWeather::Rain; }
};
