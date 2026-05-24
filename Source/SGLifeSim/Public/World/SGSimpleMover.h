#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SGSimpleMover.generated.h"

class UStaticMeshComponent;

/**
 * 极简循环移动体（第2块「活的城市」）。让占位车/行人沿一条直线匀速跑，
 * 越过终点就 wrap 回起点 —— 城市看起来「在动」，不需要寻路/避障 AI。
 *
 * 用法：spawn 后调 ConfigureMover 设方向/速度/循环长度，外部再设 mesh/材质。
 * 全占位：车是盒子、人是胶囊，美术后换皮，移动逻辑不动。
 */
UCLASS()
class SGLIFESIM_API ASGSimpleMover : public AActor
{
	GENERATED_BODY()

public:
	ASGSimpleMover();

	virtual void Tick(float DeltaSeconds) override;

	/**
	 * 配置移动：从当前位置出发，沿 Direction 方向，以 Speed(cm/s) 移动，
	 * 累计走过 LoopLength(cm) 后 wrap 回起点。Direction 会被归一化（仅水平）。
	 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|City")
	void ConfigureMover(FVector Direction, float Speed, float LoopLength);

	/** 暴露 mesh 让外部设占位外观（车身/行人）。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SGLifeSim|City")
	TObjectPtr<UStaticMeshComponent> Mesh;

private:
	FVector StartLocation = FVector::ZeroVector;
	FVector MoveDir = FVector::ForwardVector;
	float MoveSpeed = 200.f;     // cm/s
	float Loop = 10000.f;        // cm
	float Traveled = 0.f;        // 已走距离
};
