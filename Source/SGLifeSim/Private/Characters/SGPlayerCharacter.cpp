#include "Characters/SGPlayerCharacter.h"

#include "Animation/AnimSequence.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"

ASGPlayerCharacter::ASGPlayerCharacter()
{
	// 单节点 locomotion 需要每帧检查速度来切换 Idle/Walk
	PrimaryActorTick.bCanEverTick = true;

	// 等距固定视角：角色朝移动方向转，不跟随控制器朝向
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->RotationRate = FRotator(0.f, 540.f, 0.f);
	}

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 1200.f;
	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;
	CameraBoom->SetRelativeRotation(FRotator(-45.f, -45.f, 0.f));
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 10.f;

	IsometricCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("IsometricCamera"));
	IsometricCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	IsometricCamera->ProjectionMode = ECameraProjectionMode::Orthographic;
	IsometricCamera->OrthoWidth = 700.f;
	IsometricCamera->bUsePawnControlRotation = false;
}

void ASGPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
					LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (DefaultMappingContext)
				{
					Subsystem->AddMappingContext(DefaultMappingContext, 0);
				}
			}
		}
	}

	// 起步先站立
	UpdateLocomotionAnimation();
}

void ASGPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateLocomotionAnimation();
}

void ASGPlayerCharacter::UpdateLocomotionAnimation()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		return;
	}

	// 动画引用正路是 BP 子类赋值；但若为空（如 Live Coding 未刷新反射、
	// 新 UPROPERTY 尚未生效），用固定资产路径兜底加载，保证 demo 能跑。
	UAnimSequence* Idle = IdleAnim;
	UAnimSequence* Walk = WalkAnim;
	if (!Idle)
	{
		Idle = LoadObject<UAnimSequence>(nullptr, TEXT("/Game/Characters/Player/Animations/A_Idle.A_Idle"));
	}
	if (!Walk)
	{
		Walk = LoadObject<UAnimSequence>(nullptr, TEXT("/Game/Characters/Player/Animations/A_Walk.A_Walk"));
	}

	// 只看水平速度，竖直分量（落地/跳跃）不算「走」
	const float HorizontalSpeed = GetVelocity().Size2D();
	const float Threshold = (WalkSpeedThreshold > 0.f) ? WalkSpeedThreshold : 10.f;
	UAnimSequence* DesiredAnim = (HorizontalSpeed > Threshold) ? Walk : Idle;

	if (DesiredAnim && DesiredAnim != CurrentAnim)
	{
		MeshComp->PlayAnimation(DesiredAnim, /*bLooping=*/true);
		CurrentAnim = DesiredAnim;
	}
}

void ASGPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASGPlayerCharacter::Move);
		}
	}
}

void ASGPlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	// 等距视角下，2D 输入直接映射到世界 X/Y 平面：X 左右，Y 前后
	AddMovementInput(FVector(1.f, 0.f, 0.f), Axis.X);
	AddMovementInput(FVector(0.f, 1.f, 0.f), Axis.Y);
}
