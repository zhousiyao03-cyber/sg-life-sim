#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Systems/SanityTypes.h"
#include "Systems/TimeBlock.h"
#include "SanitySubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSanityChanged, int32, NewSanity, ESanityState, State);

/**
 * 理智子系统（恐怖玩法脊柱）。FSanitySystem 的 UE 薄壳。
 *
 * 理智 0~100，开局 100。恐怖事件消耗理智；每天缓慢恢复一点 —— 但**农历七月不恢复**
 *（没有喘息）。不像能量那样每日回满，以维持恐怖张力。理智越低 → 恐怖事件越频越凶。
 * 进存档。
 */
UCLASS()
class SGLIFESIM_API USanitySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Sanity")
	FOnSanityChanged OnSanityChanged;

	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Sanity")
	int32 GetSanity() const { return Sanity; }

	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Sanity")
	ESanityState GetState() const;

	/** 扣理智（恐怖事件用）。Amount 取正值表示扣多少。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Sanity")
	void Drain(int32 Amount);

	/** 恢复理智。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Sanity")
	void Restore(int32 Amount);

	/** 低理智给恐怖事件加注的额外权重。 */
	int32 GetExtraDreadWeight() const;

	/** 存档采集 / 回灌。 */
	int32 GetSanityForSave() const { return Sanity; }
	void RestoreFromSave(int32 InSanity);

private:
	UFUNCTION()
	void HandleTimeAdvanced(ETimeBlock NewBlock, int32 DayNumber);

	/** 设值并在状态档变化时广播。 */
	void SetSanity(int32 NewValue);

	int32 Sanity = 100;

	/** 最近一次每日恢复所在天，保证每天只恢复一次。 */
	int32 LastRecoveredDay = -1;

	/** 理智已归零、已触发「被压垮」结局，避免重复触发。 */
	bool bBrokenDown = false;

	/** 每日恢复量（鬼月除外）。 */
	static constexpr int32 DailyRecovery = 8;
};
