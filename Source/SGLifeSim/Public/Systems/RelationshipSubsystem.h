#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Systems/RelationshipSystem.h"
#include "Systems/RelationshipTypes.h"
#include "RelationshipSubsystem.generated.h"

/** 某 NPC 好感变化时广播给 BP。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnRelationshipChangedBP, FName, NpcId, int32, NewAffinity);

/**
 * 关系子系统。spec §6.3 + ADR 0005。
 * UE5 GameInstanceSubsystem 薄壳，内部委托给纯 C++ 的 FRelationshipSystem。
 */
UCLASS()
class SGLIFESIM_API URelationshipSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Relationship")
	void AddAffinity(FName NpcId, int32 Delta);

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Relationship")
	int32 GetAffinity(FName NpcId) const;

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Relationship")
	ERelationshipTier GetTier(FName NpcId) const;

	UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Relationship")
	FOnRelationshipChangedBP OnRelationshipChanged;

	/** 直接访问内部纯 C++ 系统（供存档用）。 */
	FRelationshipSystem& GetRelationship() { return Relationship; }
	const FRelationshipSystem& GetRelationship() const { return Relationship; }

private:
	FRelationshipSystem Relationship;
};
