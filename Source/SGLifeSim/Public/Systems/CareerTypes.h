#pragma once

#include "CoreMinimal.h"
#include "CareerTypes.generated.h"

/** 职业阶梯。spec §6.2（主业月薪逐年增长）。一个外来程序员的 CBD 升迁路。 */
UENUM(BlueprintType)
enum class ECareerLevel : uint8
{
	Unemployed UMETA(DisplayName = "待业"),
	Junior     UMETA(DisplayName = "初级工程师"),
	Mid        UMETA(DisplayName = "中级工程师"),
	Senior     UMETA(DisplayName = "高级工程师"),
	Lead       UMETA(DisplayName = "技术主管"),
	Principal  UMETA(DisplayName = "首席工程师"),
};
