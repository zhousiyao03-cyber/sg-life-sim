#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Systems/ProgressSystem.h"
#include "ProgressSubsystem.generated.h"

/** 软成就首次解锁时广播给 BP（弹窗 / 音效）。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAchievementUnlockedBP, FName, AchievementId);

/**
 * 进度子系统。spec §6.4 + ADR 0005。
 * UE5 GameInstanceSubsystem 薄壳，内部委托给纯 C++ 的 FProgressSystem。
 */
UCLASS()
class SGLIFESIM_API UProgressSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** 标记成就达成；首次达成返回 true 并广播 OnAchievementUnlocked。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Progress")
	bool MarkAchieved(FName AchievementId);

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Progress")
	bool HasAchieved(FName AchievementId) const;

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Progress")
	int32 GetAchievedCount() const;

	UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Progress")
	FOnAchievementUnlockedBP OnAchievementUnlocked;

	/** 直接访问内部纯 C++ 系统（供存档用）。 */
	FProgressSystem& GetProgress() { return Progress; }
	const FProgressSystem& GetProgress() const { return Progress; }

private:
	FProgressSystem Progress;
};
