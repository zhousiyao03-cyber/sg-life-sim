#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WantedSubsystem.generated.h"

/** 通缉星级变化广播（HUD 显示几颗星）。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWantedChanged, int32, Stars);

/**
 * 通缉值系统（第9块 GTA 街头）。打人/犯事累积通缉值，换算成 0~5 星。
 * 警察 NPC 查通缉星级决定是否追捕。一段时间不犯事自动消退。
 *
 * 纯逻辑 GameInstanceSubsystem，跨关卡保留。
 */
UCLASS()
class SGLIFESIM_API UWantedSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 增加通缉值（打人 +25，杀人 +100 之类）。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Wanted")
	void AddHeat(int32 Amount);

	/** 当前通缉星级 0~5。 */
	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Wanted")
	int32 GetStars() const;

	/** 当前通缉值原始分。 */
	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Wanted")
	int32 GetHeat() const { return Heat; }

	/** 清零（被捕 / 用钱消通缉）。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Wanted")
	void ClearWanted();

	/** 时间推进时调，通缉值自然消退。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Wanted")
	void Decay(int32 Amount);

	UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Wanted")
	FOnWantedChanged OnWantedChanged;

private:
	void BroadcastIfChanged(int32 OldStars);

	int32 Heat = 0; // 0..500，每 100 一星
};
