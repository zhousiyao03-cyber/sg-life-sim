#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Systems/TimeBlock.h"
#include "SGDayNightController.generated.h"

class ADirectionalLight;
class ASkyLight;
class AExponentialHeightFog;

/**
 * 昼夜光照控制器（第5块）。放进城市 / 室外关卡里的一个 Actor。
 *
 * BeginPlay 时找场景里的 DirectionalLight / SkyLight / ExponentialHeightFog，
 * 订阅 UTimeSubsystem::OnTimeAdvanced，按当前时间块（早/上午/下午/晚/深夜）
 * 调太阳角度、强度、颜色、雾浓度 —— 让深夜真的是深夜，接通恐怖氛围
 * （鬼月深夜恐怖事件触发时画面是黑/冷/雾，而不是大白天）。
 *
 * 没有美术天空盒，靠 SkyAtmosphere + 动态方向光出昼夜，全占位。
 */
UCLASS()
class SGLIFESIM_API ASGDayNightController : public AActor
{
	GENERATED_BODY()

public:
	ASGDayNightController();

protected:
	virtual void BeginPlay() override;

	/** 订阅时间推进：时间块变了就重设光照。 */
	UFUNCTION()
	void HandleTimeAdvanced(ETimeBlock NewBlock, int32 DayNumber);

	/** 按时间块把场景光照（太阳/天空/雾）设成对应调性。 */
	void ApplyLighting(ETimeBlock Block);

	/** 缺失的光照 Actor 没有就不动它（关卡可能没放全）。 */
	UPROPERTY(Transient)
	TObjectPtr<ADirectionalLight> Sun;

	UPROPERTY(Transient)
	TObjectPtr<ASkyLight> Sky;

	UPROPERTY(Transient)
	TObjectPtr<AExponentialHeightFog> Fog;
};
