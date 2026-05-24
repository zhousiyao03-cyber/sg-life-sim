#pragma once

#include "CoreMinimal.h"
#include "RelationshipTypes.generated.h"

/**
 * 关系等级。spec §6.3（轻度 Persona 5 风）。
 * 由好感度 0~100 映射而来；阈值见 FRelationshipSystem::GetTier。
 */
UENUM(BlueprintType)
enum class ERelationshipTier : uint8
{
	Stranger      UMETA(DisplayName = "陌生"),   // 0~9
	Acquaintance  UMETA(DisplayName = "认识"),   // 10~29
	Familiar      UMETA(DisplayName = "熟悉"),   // 30~49
	Friend        UMETA(DisplayName = "朋友"),   // 50~69
	Confidant     UMETA(DisplayName = "知己"),   // 70~89
	Lover         UMETA(DisplayName = "恋人"),   // 90~100
};
