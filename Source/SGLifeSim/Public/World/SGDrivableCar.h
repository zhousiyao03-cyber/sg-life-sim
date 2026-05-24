#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Interactables/InteractableInterface.h"
#include "SGDrivableCar.generated.h"

class UStaticMeshComponent;
class UCameraComponent;
class USpringArmComponent;
class UInputComponent;
struct FInputActionValue;

/**
 * 可驾驶的车（第7块）。简化驾驶，不走 Chaos 物理载具（避免依赖载具骨架资产）：
 * 占位盒子车身 + 自写的前进/转向逻辑 + 车后第三人称相机。
 *
 * 上车：玩家走近按 E（IInteractableInterface）→ controller possess 这辆车。
 * 开车：W/S 油门、A/D 转向（移动时才能转），相机在车后。
 * 下车：开车时按 E → controller possess 回玩家角色，角色挪到车旁。
 *
 * 全占位，移动逻辑不依赖美术；换真车模型只换 mesh。
 */
UCLASS()
class SGLIFESIM_API ASGDrivableCar : public APawn, public IInteractableInterface
{
	GENERATED_BODY()

public:
	ASGDrivableCar();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Car")
	TObjectPtr<UStaticMeshComponent> Body;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Car")
	TObjectPtr<USpringArmComponent> CameraArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Car")
	TObjectPtr<UCameraComponent> Camera;

private:
	void Throttle(float Value); // W/S 前后（legacy 轴）
	void Steer(float Value);     // A/D 转向（legacy 轴）
	void ExitCar();              // E 下车

	/** 上车前记住的角色（下车 possess 回去）。 */
	UPROPERTY(Transient)
	TObjectPtr<APawn> DriverPawn;

	float ThrottleInput = 0.f;
	float SteerInput = 0.f;
	float MaxSpeed = 1400.f;   // cm/s（~50km/h）
	float CurrentSpeed = 0.f;
	float Accel = 800.f;       // cm/s^2
	float TurnRate = 90.f;     // deg/s（满速时）
};
