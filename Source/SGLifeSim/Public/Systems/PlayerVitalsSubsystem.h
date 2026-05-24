#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PlayerVitalsSubsystem.generated.h"

/** 血量变化广播（HUD 画血条）。Current/Max。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, int32, Current, int32, Max);

/** 玩家死亡广播：玩家角色订阅后做重生传送。携带「重生点」语义由订阅方决定。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDied);

/** 玩家重生完成广播（数值已复位、代价已扣），用于弹「在医院醒来…」气泡。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerRespawned, int32, HospitalFeeCents);

/**
 * 玩家生命值系统壳（B 块 GTA）。包 FPlayerVitalsSystem 纯核心，跨关卡保留。
 *
 * 受击扣血→归零死亡→GTA 式「医院重生」：血量回满、扣一笔医院费（现金不足走银行）、
 * 清掉通缉，然后广播 OnPlayerDied 让玩家角色把自己传送回出生点。
 */
UCLASS()
class SGLIFESIM_API UPlayerVitalsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** 当前血量 0~100。 */
	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Vitals")
	int32 GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Vitals")
	int32 GetMaxHealth() const;

	/** 受到伤害（NPC 反击/枪击调）。归零则走死亡重生流程。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Vitals")
	void ApplyDamage(int32 Damage);

	/** 回血（休息/医疗）。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Vitals")
	void Heal(int32 Amount);

	/** 直接设血量（读档恢复用）。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Vitals")
	void SetHealth(int32 NewHealth);

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Vitals")
	bool IsDead() const;

	/** 死亡重生的医院费（分）。GTA 式：死一次破点财。 */
	static constexpr int64 HospitalFeeCents = 50000; // S$500

	UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Vitals")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Vitals")
	FOnPlayerDied OnPlayerDied;

	UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Vitals")
	FOnPlayerRespawned OnPlayerRespawned;

private:
	/** 死亡处理：扣医院费、清通缉、血量回满，广播 Died + Respawned。 */
	void Die();

	int32 Health = 100;
};
