#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SGPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UAnimSequence;
struct FInputActionValue;

/**
 * 主角基类。spec §10.3「C++ 核心 + Blueprint 薄壳」。
 *
 * 移动（Enhanced Input WASD）与等距俯视相机都在 C++ 实现；Blueprint 子类
 * （BP_PlayerCharacter）只负责设 SkeletalMesh / AnimClass，并把 IMC/IA 资产
 * 引用赋给下面暴露的属性。
 */
UCLASS()
class SGLIFESIM_API ASGPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASGPlayerCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** 等距相机吊臂。Pitch=-45/Yaw=-45 固定角度，正交投影。ADR 0003。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Camera")
	TObjectPtr<UCameraComponent> IsometricCamera;

	/** 在 BP 子类里赋值为 IMC_Default。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** 在 BP 子类里赋值为 IA_Move（Axis2D）。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Input")
	TObjectPtr<UInputAction> MoveAction;

	/**
	 * 单节点 locomotion（spec §10.3）：不挂 AnimBlueprint，直接在 C++ 里按速度
	 * 切换播放下面两个序列。BP 子类把它们赋为 A_Idle / A_Walk。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Animation")
	TObjectPtr<UAnimSequence> IdleAnim;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Animation")
	TObjectPtr<UAnimSequence> WalkAnim;

	/** 速度高于此阈值视为「在走」，低于则「站立」。cm/s。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Animation")
	float WalkSpeedThreshold = 10.f;

private:
	/** IA_Move 的 Triggered 回调：把 2D 输入映射到世界 X/Y 平面移动。 */
	void Move(const FInputActionValue& Value);

	/** 按当前水平速度在 Idle / Walk 间切换单节点播放，避免每帧重复 Play。 */
	void UpdateLocomotionAnimation();

	/** 返回交互范围内最近的可交互 Actor（无则 nullptr）。交互与 HUD 提示共用。 */
	AActor* FindNearbyInteractable() const;

	/** IA_Interact（E）：找范围内最近的可交互对象并触发 OnInteract。 */
	void TryInteract();

	/** IA_AdvanceTime（T）：推进一个时间块（经 UTimeSubsystem）。 */
	void AdvanceTime();

	/** IA_OpenLocationMenu（M）：在出租屋 / 食阁两个关卡间切换。原型用直接跳转代替菜单 UMG。 */
	void SwitchLocation();

	/** 每帧把「Day X · 周几 · 时间块」画到屏幕（原型 HUD，替代 UMG W_HUD）。 */
	void DrawPrototypeHUD();

	/** 缓存当前正在播的序列，状态没变就不重新 Play。 */
	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> CurrentAnim;
};
