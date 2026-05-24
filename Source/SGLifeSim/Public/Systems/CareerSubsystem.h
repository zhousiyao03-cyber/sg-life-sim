#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Systems/CareerSystem.h"
#include "Systems/CareerTypes.h"
#include "Systems/TimeBlock.h"
#include "CareerSubsystem.generated.h"

/** 职业状态变化（升职/跳槽/薪资变动）时广播，供 HUD 刷新。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCareerChanged);

/**
 * 职业子系统。spec §6.2 + ADR 0005。
 *
 * UE5 GameInstanceSubsystem 薄壳，内部委托给纯 C++ 的 FCareerSystem。
 * 与 Economy 接线：把当前税前月薪推给 UEconomySubsystem，月度结算照常发薪（含 CPF）。
 * 订阅 TimeSubsystem 月初累计在职时长；升职读 PlayerState 的专业技能。
 */
UCLASS()
class SGLIFESIM_API UCareerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Career")
	ECareerLevel GetLevel() const { return Career.GetLevel(); }

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Career")
	int64 GetGrossSalaryCents() const { return Career.GetGrossSalaryCents(); }

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Career")
	int32 GetMonthsInRole() const { return Career.GetMonthsInRole(); }

	/** 当前是否够格升职（读 PlayerState 专业技能）。 */
	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Career")
	bool CanPromote() const;

	/** 申请升职：满足条件则升级涨薪、推薪资给 Economy、首次成功标成就。返回是否升职。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Career")
	bool TryPromote();

	/** 跳槽：薪资 +RaisePercent%（默认 35），推薪资给 Economy。待业不可跳槽。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Career")
	bool JobHop(int32 RaisePercent = 35);

	UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Career")
	FOnCareerChanged OnCareerChanged;

	FCareerSystem& GetCareer() { return Career; }
	const FCareerSystem& GetCareer() const { return Career; }

	/** 把当前月薪推给 Economy（薪资变化 / 读档后调用，保持发薪同步）。 */
	void SyncSalaryToEconomy();

private:
	FCareerSystem Career;

	/** 上次累计在职的月号，检测跨月。 */
	int32 LastTickedMonth = 1;

	/** 当前主角专业技能（读 PlayerState，缺省 0）。 */
	int32 GetProfessional() const;

	UFUNCTION()
	void HandleTimeAdvanced(ETimeBlock NewBlock, int32 DayNumber);
};
