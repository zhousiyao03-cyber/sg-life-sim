#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SGPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
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

private:
	/** IA_Move 的 Triggered 回调：把 2D 输入映射到世界 X/Y 平面移动。 */
	void Move(const FInputActionValue& Value);
};
