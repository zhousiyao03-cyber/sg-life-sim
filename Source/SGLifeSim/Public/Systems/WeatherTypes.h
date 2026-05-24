#pragma once

#include "CoreMinimal.h"
#include "WeatherTypes.generated.h"

/** 天气（H 块 GTA）。 */
UENUM(BlueprintType)
enum class EWeather : uint8
{
	Clear  UMETA(DisplayName = "晴"),
	Cloudy UMETA(DisplayName = "多云"),
	Rain   UMETA(DisplayName = "雨"),
	Fog    UMETA(DisplayName = "雾"),
};
