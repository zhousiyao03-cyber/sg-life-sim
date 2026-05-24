#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Systems/EconomySystem.h"
#include "Systems/EconomyTypes.h"
#include "Systems/TimeBlock.h"
#include "EconomySubsystem.generated.h"

/** 某账户余额变化时广播。NewBalanceCents 是该账户变化后的余额（分）。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnBalanceChanged, ECurrencyAccount, Account, int64, NewBalanceCents);

/**
 * 经济子系统。spec §6.2 + ADR 0005。
 *
 * UE5 GameInstanceSubsystem 的薄包装层 —— 内部委托给纯 C++ 的 FEconomySystem。
 * 职责：
 *   - 把 FEconomySystem 的 API 暴露给 Blueprint（金额以「分」传递）
 *   - 余额变化时广播 OnBalanceChanged
 *   - 跨场景持久（GameInstance 生命周期），与 UTimeSubsystem 同模式
 *
 * Blueprint 访问方式：Get Game Instance Subsystem → EconomySubsystem
 */
UCLASS()
class SGLIFESIM_API UEconomySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// UGameInstanceSubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Economy")
	int64 GetBalance(ECurrencyAccount Account) const;

	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Economy")
	void Deposit(ECurrencyAccount Account, int64 Cents, FName Reason);

	/** 取款。余额不足返回 false 且不改动任何状态。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Economy")
	bool TryWithdraw(ECurrencyAccount Account, int64 Cents, FName Reason);

	/** 按月发薪（含 CPF 分账）。GrossCents = 税前月薪（分）。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Economy")
	void ApplyMonthlySalary(int64 GrossCents);

	/** 设置月度结算用的税前月薪（分）。由 UCareerSubsystem 在薪资变化时推入。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Economy")
	void SetMonthlyGrossSalary(int64 GrossCents) { MonthlyFinance.SalaryGrossCents = FMath::Max((int64)0, GrossCents); }

	/** 当前月度结算用的税前月薪（分）。 */
	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Economy")
	int64 GetMonthlyGrossSalary() const { return MonthlyFinance.SalaryGrossCents; }

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Economy")
	int64 GetNetWorth() const;

	/** 订阅这个 delegate 接收余额变化通知（如刷新 HUD 钱包显示）。 */
	UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Economy")
	FOnBalanceChanged OnBalanceChanged;

	/** 月度收支配置（工资 + 固定账单）。数据驱动，可在 BP / 运行时调。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SGLifeSim|Economy")
	FMonthlyFinance MonthlyFinance;

	/** 直接访问内部纯 C++ 系统（供其他 C++ 系统/存档用，不暴露给 BP）。 */
	FEconomySystem& GetEconomy() { return Economy; }
	const FEconomySystem& GetEconomy() const { return Economy; }

	/** 立即跑一次月度结算（发薪 + 扣固定账单）。月初由时间事件自动触发，也可手动调。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Economy")
	void RunMonthlySettlement();

private:
	FEconomySystem Economy;

	/** 上一次已结算的月号，用于检测跨月（避免同月重复发薪）。 */
	int32 LastSettledMonth = 1;

	/** 绑定到 UTimeSubsystem::OnTimeAdvanced；跨入新月时触发结算。 */
	UFUNCTION()
	void HandleTimeAdvanced(ETimeBlock NewBlock, int32 DayNumber);

	/** 广播受影响账户的最新余额。 */
	void NotifyBalance(ECurrencyAccount Account);
};
