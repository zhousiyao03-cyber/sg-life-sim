#pragma once

#include "CoreMinimal.h"
#include "AssetsTypes.generated.h"

/** 住房阶梯。spec §6.4 资产。 */
UENUM(BlueprintType)
enum class EHousingTier : uint8
{
	None         UMETA(DisplayName = "无"),
	RentedRoom   UMETA(DisplayName = "出租屋单间"),
	RentedFlat   UMETA(DisplayName = "整租组屋"),
	OwnedHDB     UMETA(DisplayName = "自购组屋"),
	OwnedCondo   UMETA(DisplayName = "自购公寓"),
	Multiple     UMETA(DisplayName = "多套房产"),
};

/** 车辆阶梯。spec §6.4 资产。 */
UENUM(BlueprintType)
enum class EVehicleTier : uint8
{
	None       UMETA(DisplayName = "无"),
	GrabPass   UMETA(DisplayName = "Grab 月卡"),
	UsedCar    UMETA(DisplayName = "二手车"),
	NewCar     UMETA(DisplayName = "新车"),
	LuxuryCar  UMETA(DisplayName = "豪车"),
};
