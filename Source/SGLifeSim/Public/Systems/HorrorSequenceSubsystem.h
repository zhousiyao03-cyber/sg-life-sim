#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Systems/HorrorSceneTypes.h"
#include "HorrorSequenceSubsystem.generated.h"

/** 送回原关卡、结算完毕后广播事后文案（UI 弹气泡）。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHorrorSceneAftermath, const FText&, Message);

/**
 * 恐怖场景演出总控（Plan 24）。GameInstanceSubsystem —— 跨关卡存活，
 * 因为「去 / 演 / 回」要横跨两次 OpenLevel。
 *
 * EnterScene：记下来源关卡 + 玩家坐标，置 bInScene，OpenLevel 进恐怖关卡。
 * 关卡内 AElevatorHorrorDirector 跑演出，演完调 ExitScene。
 * ExitScene：结算（扣理智 + 记图鉴 + 广播事后文案），OpenLevel 回来源关卡。
 *
 * 防重入：演出途中（bInScene）拒绝新的 EnterScene。
 * 见 docs/superpowers/specs/2026-05-24-horror-scene-sequence-design.md。
 */
UCLASS()
class SGLIFESIM_API UHorrorSequenceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 送回 + 结算后广播事后文案。 */
	UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Horror")
	FOnHorrorSceneAftermath OnAftermath;

	/**
	 * 进入一个恐怖场景演出。记来源关卡 + 玩家坐标，OpenLevel 过去。
	 * @return 成功进入返回 true；已在演出中（防重入）或场景无效返回 false。
	 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Horror")
	bool EnterScene(EHorrorScene Scene);

	/**
	 * 演出结束：结算（扣理智 + 记图鉴 + 广播事后文案），OpenLevel 回来源关卡。
	 * 由关卡内导演演完调用。不在演出中则忽略。
	 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Horror")
	void ExitScene();

	/** 当前正在演的场景（None=不在演出中）。供恐怖关卡的导演 BeginPlay 查询。 */
	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Horror")
	EHorrorScene GetActiveScene() const { return ActiveScene; }

	/** 是否正处于恐怖场景演出中。 */
	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Horror")
	bool IsInScene() const { return bInScene; }

	/**
	 * 结算逻辑独立出来供集成测试直接验证（不依赖 OpenLevel 这种运行时行为）。
	 * 扣理智 + 记图鉴 + 广播事后文案。返回结算用的场景定义。
	 */
	FHorrorSceneDef ResolveOutcome(EHorrorScene Scene);

private:
	/** 正在演的场景。 */
	EHorrorScene ActiveScene = EHorrorScene::None;

	/** 防重入标志。 */
	bool bInScene = false;

	/** 来源关卡名（演完回这里）。 */
	FName ReturnLevelName;

	/** 玩家离开时的世界坐标 / 朝向（第一版先存不强制还原）。 */
	FVector ReturnLocation = FVector::ZeroVector;
	FRotator ReturnRotation = FRotator::ZeroRotator;
};
