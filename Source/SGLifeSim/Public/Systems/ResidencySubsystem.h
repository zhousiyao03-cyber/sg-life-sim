#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Systems/ResidencySystem.h"
#include "Systems/ResidencyTypes.h"
#include "ResidencySubsystem.generated.h"

/** 身份变化时广播。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResidencyChanged, EResidencyStatus, NewStatus);

/**
 * 居留身份子系统。spec §6.4 + ADR 0005。
 * UE5 GameInstanceSubsystem 薄壳，内部委托给纯 C++ 的 FResidencySystem。
 */
UCLASS()
class SGLIFESIM_API UResidencySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Residency")
	EResidencyStatus GetStatus() const { return Residency.GetStatus(); }

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Residency")
	int32 GetPRRejectionCount() const { return Residency.GetPRRejectionCount(); }

	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Residency")
	bool ApplyForPR();

	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Residency")
	bool ResolvePRApplication(bool bApproved);

	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Residency")
	bool Naturalize();

	UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Residency")
	FOnResidencyChanged OnResidencyChanged;

	FResidencySystem& GetResidency() { return Residency; }
	const FResidencySystem& GetResidency() const { return Residency; }

private:
	FResidencySystem Residency;
};
