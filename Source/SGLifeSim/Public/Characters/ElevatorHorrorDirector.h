#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Systems/ElevatorSequenceBeats.h"
#include "ElevatorHorrorDirector.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class APawn;

/**
 * 电梯空楼层恐怖演出导演（Plan 24）。放在 L_ElevatorHorror 关卡里的一个 Actor。
 *
 * BeginPlay 时若 UHorrorSequenceSubsystem 正在演电梯场景，则接管：
 *  - 锁玩家（禁移动 / 禁交互，保留转头）
 *  - 代码 spawn 占位道具（电梯壳 / 门 / 剪影女鬼 / 顶灯）——资产到位后换 mesh 引用，时间线不动
 *  - 按 FElevatorSequenceBeats 用 FTimerManager 排定时回调，逐节拍执行
 *  - 末节拍 Exit 调 Subsystem::ExitScene() 送回原关卡
 *
 * 见 docs/superpowers/specs/2026-05-24-horror-scene-sequence-design.md。
 */
UCLASS()
class SGLIFESIM_API AElevatorHorrorDirector : public AActor
{
	GENERATED_BODY()

public:
	AElevatorHorrorDirector();

protected:
	virtual void BeginPlay() override;

	/** 电梯轿厢（占位：拉伸的盒子）。 */
	UPROPERTY(VisibleAnywhere, Category = "SGLifeSim|Horror")
	TObjectPtr<UStaticMeshComponent> CarMesh;

	/** 电梯门（占位：盒子，开门时代码 Lerp 平移）。 */
	UPROPERTY(VisibleAnywhere, Category = "SGLifeSim|Horror")
	TObjectPtr<UStaticMeshComponent> DoorMesh;

	/** 女鬼剪影（占位：黑色人形面片 / 盒子，平时隐藏）。资产到位后换 Meshy 女鬼模型。 */
	UPROPERTY(VisibleAnywhere, Category = "SGLifeSim|Horror")
	TObjectPtr<UStaticMeshComponent> GhostMesh;

	/** 电梯顶灯（闪烁 / 爆闪 / 灭）。 */
	UPROPERTY(VisibleAnywhere, Category = "SGLifeSim|Horror")
	TObjectPtr<UPointLightComponent> CeilingLight;

private:
	/** 排定整段演出的所有定时回调。 */
	void ScheduleSequence();

	/** 执行单个节拍。 */
	void RunBeat(EElevatorBeat Beat);

	/** 锁 / 解锁玩家（禁移动与交互，保留视角转动）。 */
	void LockPlayer(bool bLock);

	/** 每个节拍一个 timer handle，避免被覆盖。 */
	TArray<FTimerHandle> BeatTimers;

	/** 顶灯基准强度（恢复用）。 */
	float BaseLightIntensity = 5000.f;

	/** 是否已接管（Subsystem 确认在演电梯场景）。未接管则不动玩家、不演。 */
	bool bActive = false;
};
