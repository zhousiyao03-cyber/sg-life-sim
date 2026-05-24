#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Systems/NightCommuteTypes.h"
#include "Math/RandomStream.h"
#include "NightCommuteSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNightCommuteResolved, const FText&, Message);

/**
 * 鬼月夜归抉择子系统（Plan 23）。FNightCommuteSystem 的 UE 薄壳。
 *
 * 第一个「玩家主动面对恐怖、承担选择后果」的桥段。仅在农历七月深夜（LateNight）可触发，
 * 玩家选「等下一趟 / 赶紧进去 / 走楼梯」，结算改理智/精力，赌输还会真出事（重扣理智）。
 * 注入种子可复现。
 */
UCLASS()
class SGLIFESIM_API UNightCommuteSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** 结算后广播文案（UI 弹气泡）。 */
	UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Horror")
	FOnNightCommuteResolved OnResolved;

	/** 当前是否可触发夜归抉择（农历七月 + 深夜）。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Horror")
	bool IsAvailable() const;

	/** 做出抉择并结算。返回结果（文案 + 理智/精力变化 + 是否出事）。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Horror")
	FNightCommuteOutcome MakeChoice(ENightCommuteChoice Choice);

	/** 设种子（复现/测试用）。 */
	void SetSeed(int32 Seed) { Stream.Initialize(Seed); }

private:
	FRandomStream Stream;
};
