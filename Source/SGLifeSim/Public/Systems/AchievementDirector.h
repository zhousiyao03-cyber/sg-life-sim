#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Systems/EconomyTypes.h"
#include "AchievementDirector.generated.h"

/**
 * 成就导演。Plan 3 Task 3。
 *
 * 跨系统协调者：订阅经济 / 关系子系统的变化委托，按 spec §6.4 的软成就规则
 * 判断是否达成，达成则调 UProgressSubsystem::MarkAchieved（首次解锁会经
 * OnAchievementUnlocked 广播 → HUD toast）。
 *
 * 这样经济 / 关系 / 进度三者互不依赖，「什么算成就」只活在这一个地方。
 */
UCLASS()
class SGLIFESIM_API UAchievementDirector : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	/** 经济余额变化 → 判断「首次发薪」「净资产 $10k」。 */
	UFUNCTION()
	void HandleBalanceChanged(ECurrencyAccount Account, int64 NewBalanceCents);

	/** 关系好感变化 → 判断「第一个朋友」。 */
	UFUNCTION()
	void HandleRelationshipChanged(FName NpcId, int32 NewAffinity);

	/** 经济相关成就的统一检查（发薪流水 / 净资产阈值）。 */
	void EvaluateEconomyAchievements();
};
