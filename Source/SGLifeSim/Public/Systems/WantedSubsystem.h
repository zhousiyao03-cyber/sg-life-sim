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

	/**
	 * 目击者举报：多个路人看到同一桩事只该报一次，不是每人各加一笔。
	 * 全局限频——距上次举报不足 ReportCooldownSeconds 则忽略（吞掉重复目击）。
	 * 返回是否真的记了一笔（供调用方决定要不要播报警音等）。
	 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Wanted")
	bool ReportCrime(int32 Amount, float NowSeconds);

	/**
	 * 被捕：交一笔保释金（现金不够从银行扣，再不够能扣多少扣多少）后清通缉。
	 * 区别于 ClearWanted（无代价清零，仅供读档/死亡医院流程用）——警察抓到你是要付代价的。
	 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Wanted")
	void Arrest();

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

	/** 读档恢复通缉值。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Wanted")
	void RestoreFromSave(int32 InHeat);

	UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Wanted")
	FOnWantedChanged OnWantedChanged;

	/** 被捕扣的保释金（分）：和警察局自首同价，让"被抓"和"自首"代价一致。 */
	static constexpr int64 ArrestBailCents = 20000; // S$200

private:
	void BroadcastIfChanged(int32 OldStars);

	int32 Heat = 0; // 0..500，每 100 一星

	/** 上次目击举报的时刻（秒，世界时间）。用于全局去重，避免一群路人各报一笔。 */
	float LastReportSeconds = -1000.f;
	/** 目击举报全局冷却（秒）：此窗口内的重复举报都吞掉。 */
	static constexpr float ReportCooldownSeconds = 3.f;
};
