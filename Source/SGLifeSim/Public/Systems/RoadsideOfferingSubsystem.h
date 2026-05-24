#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Systems/RoadsideOfferingTypes.h"
#include "Math/RandomStream.h"
#include "RoadsideOfferingSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoadsideOfferingResolved, const FText&, Message);

/**
 * 鬼月路边祭品抉择子系统（Plan 26）。FRoadsideOfferingSystem 的 UE 薄壳。
 *
 * 与夜归电梯抉择同构的第二个主动博弈，仅农历七月深夜可触发。
 * 玩家选「绕开 / 跨过去 / 拜一拜」，结算改理智/精力；跨过去赌输则犯了冥纸禁忌——
 * 接进恐怖事件层（ApplyEvent ZhiQianTaboo：扣属性 + 进图鉴 + 广播），与 AhMei 的冥纸共鸣对话闭环。
 * 注入种子可复现。
 */
UCLASS()
class SGLIFESIM_API URoadsideOfferingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** 结算后广播文案（UI 弹气泡）。 */
	UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Horror")
	FOnRoadsideOfferingResolved OnResolved;

	/** 当前是否可触发路边祭品抉择（农历七月 + 深夜）。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Horror")
	bool IsAvailable() const;

	/** 做出抉择并结算。返回结果（文案 + 理智/精力变化 + 是否出事）。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Horror")
	FRoadsideOfferingOutcome MakeChoice(ERoadsideOfferingChoice Choice);

	/** 设种子（复现/测试用）。 */
	void SetSeed(int32 Seed) { Stream.Initialize(Seed); }

private:
	FRandomStream Stream;
};
