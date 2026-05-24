#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Interactables/InteractableInterface.h"
#include "SGHelicopter.generated.h"

class UStaticMeshComponent;
class UCameraComponent;
class USpringArmComponent;
class UInputComponent;

/**
 * 可驾驶直升机（H 块 GTA）。沿用 DrivableCar 的简化思路（不走 Chaos 载具），
 * 占位机身盒子 + 顶部旋翼盘 + 机后第三人称相机 + 自写飞行逻辑。
 *
 * 上机：走近按 E（IInteractableInterface）→ possess。
 * 飞：W/S 前后、A/D 偏航转向、空格上升 / 左 Ctrl 下降（HeliUp 轴）。E 下机。
 * 与车的区别：有垂直维度（升降），且无重力（停桨悬停由玩家自己控）。
 *
 * 全占位，飞行逻辑不依赖美术；换真直升机模型只换 mesh，旋翼自转是占位演出。
 */
UCLASS()
class SGLIFESIM_API ASGHelicopter : public APawn, public IInteractableInterface
{
	GENERATED_BODY()

public:
	ASGHelicopter();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Heli")
	TObjectPtr<UStaticMeshComponent> Body;

	/** 顶部旋翼盘（占位，Tick 里自转出「在转」的感觉）。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Heli")
	TObjectPtr<UStaticMeshComponent> Rotor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Heli")
	TObjectPtr<USpringArmComponent> CameraArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Heli")
	TObjectPtr<UCameraComponent> Camera;

private:
	void Throttle(float Value); // W/S 前后（legacy 轴 CarThrottle）
	void Steer(float Value);    // A/D 偏航（legacy 轴 CarSteer）
	void Lift(float Value);     // 空格/Ctrl 升降（legacy 轴 HeliUp）
	void ExitHeli();            // E 下机

	UPROPERTY(Transient)
	TObjectPtr<APawn> DriverPawn;

	float ThrottleInput = 0.f;
	float SteerInput = 0.f;
	float LiftInput = 0.f;

	float MaxSpeed = 2200.f;   // cm/s 前进
	float CurrentSpeed = 0.f;
	float Accel = 1000.f;
	float TurnRate = 70.f;     // deg/s 偏航
	float LiftSpeed = 900.f;   // cm/s 升降
	float RotorSpinRate = 720.f; // deg/s 旋翼自转
};
