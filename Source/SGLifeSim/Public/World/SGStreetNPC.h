#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SGStreetNPC.generated.h"

class UStaticMeshComponent;

/** 街头 NPC 类型。 */
UENUM(BlueprintType)
enum class EStreetNpcKind : uint8
{
	Pedestrian UMETA(DisplayName = "路人"),
	Police     UMETA(DisplayName = "警察"),
	Gangster   UMETA(DisplayName = "帮派"),
};

/**
 * 街头 NPC（第9块 GTA）。占位胶囊 + 血量 + 简单行为：
 *  - 路人：被打掉血，死了倒地（隐藏）。
 *  - 警察：玩家通缉星 >0 时朝玩家移动追捕，靠近就「逮捕」（清通缉）。
 *  - 帮派：见玩家就敌对靠近（占位行为同警察追，但不清通缉）。
 *
 * 追捕用 UE 导航系统寻路（FindPathToLocationSynchronously）绕开建筑，沿路径点走，
 * 不再直线穿墙。无 NavMesh 时回退直线（保底）。受击/死亡动画待美术。
 */
UCLASS()
class SGLIFESIM_API ASGStreetNPC : public AActor
{
	GENERATED_BODY()

public:
	ASGStreetNPC();

	virtual void Tick(float DeltaSeconds) override;

	/** 配置类型（生成时设）。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Street")
	void ConfigureKind(EStreetNpcKind InKind);

	/** 受到伤害（玩家出拳调）。掉血、死亡。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Street")
	void TakeMeleeHit(int32 Damage);

	EStreetNpcKind GetKind() const { return Kind; }
	bool IsDead() const { return bDead; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Street")
	TObjectPtr<UStaticMeshComponent> Body;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Street")
	TObjectPtr<UStaticMeshComponent> Head;

private:
	/** 找玩家 Pawn（追捕用）。 */
	AActor* FindPlayer() const;

	/** 朝目标走一步：优先沿 NavMesh 路径折线，无导航则回退直线。 */
	void ChaseTowards(const FVector& TargetLocation, float DeltaSeconds);

	EStreetNpcKind Kind = EStreetNpcKind::Pedestrian;
	int32 Health = 100;
	bool bDead = false;
	float ChaseSpeed = 280.f; // cm/s

	/** 当前缓存的寻路折线（世界坐标点）。 */
	TArray<FVector> PathPoints;
	/** 当前正前往 PathPoints 的哪个点。 */
	int32 PathIndex = 0;
	/** 距下次重算路径还剩多久（秒），目标在移动所以要周期性刷新。 */
	float RepathCooldown = 0.f;

	/** 近战攻击冷却（秒），帮派贴近玩家时按此节奏出手扣血。 */
	float AttackCooldown = 0.f;
};
