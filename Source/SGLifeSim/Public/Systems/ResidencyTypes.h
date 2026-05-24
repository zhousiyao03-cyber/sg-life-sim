#pragma once

#include "CoreMinimal.h"
#include "ResidencyTypes.generated.h"

/**
 * 居留身份阶梯。spec §6.4 身份。新加坡外来者的主线脊柱。
 * 用 uint8 因为 UEnum + UPROPERTY（Subsystem / 存档 / BP）。
 */
UENUM(BlueprintType)
enum class EResidencyStatus : uint8
{
	WorkPermit_EP  UMETA(DisplayName = "工作准证 EP"),
	WorkPermit_SP  UMETA(DisplayName = "工作准证 SP"),
	PR_Applying    UMETA(DisplayName = "PR 申请中"),
	PR             UMETA(DisplayName = "永久居民 PR"),
	Citizen        UMETA(DisplayName = "公民"),
};
