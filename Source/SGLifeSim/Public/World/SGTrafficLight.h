#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SGTrafficLight.generated.h"

class UStaticMeshComponent;

/** 信号灯相位。 */
UENUM(BlueprintType)
enum class ETrafficPhase : uint8
{
	Green  UMETA(DisplayName = "绿"),
	Yellow UMETA(DisplayName = "黄"),
	Red    UMETA(DisplayName = "红"),
};

/**
 * 交通信号灯（F 块 GTA）。纯计时状态机：绿→黄→红→绿 循环。
 * 占位灯柱 + 顶部一个会变色的灯头盒子。车（ASGSimpleMover）查附近灯的相位决定停走。
 *
 * 同一路口对向两组灯相位相反（一组绿时另一组红），由生成时设 InitialPhase 错开。
 */
UCLASS()
class SGLIFESIM_API ASGTrafficLight : public AActor
{
	GENERATED_BODY()

public:
	ASGTrafficLight();

	virtual void Tick(float DeltaSeconds) override;

	/** 配置：起始相位（错开对向）、各相位时长。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Traffic")
	void ConfigureLight(ETrafficPhase InitialPhase, float GreenSeconds = 6.f, float YellowSeconds = 2.f, float RedSeconds = 8.f);

	/** 当前相位。 */
	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Traffic")
	ETrafficPhase GetPhase() const { return Phase; }

	/** 车应否停（红或黄都停）。 */
	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Traffic")
	bool ShouldVehiclesStop() const { return Phase != ETrafficPhase::Green; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Traffic")
	TObjectPtr<UStaticMeshComponent> Pole;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Traffic")
	TObjectPtr<UStaticMeshComponent> Head;

private:
	/** 进入某相位：更新计时与灯头颜色。 */
	void EnterPhase(ETrafficPhase NewPhase);

	/** 按相位给灯头换占位色材质（绿/黄/红）。 */
	void ApplyHeadColor();

	ETrafficPhase Phase = ETrafficPhase::Green;
	float PhaseTimer = 0.f;
	float GreenTime = 6.f;
	float YellowTime = 2.f;
	float RedTime = 8.f;
};
