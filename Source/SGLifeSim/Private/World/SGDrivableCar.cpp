#include "World/SGDrivableCar.h"

#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/GameInstance.h"
#include "Systems/SGAudioSubsystem.h"

ASGDrivableCar::ASGDrivableCar()
{
	PrimaryActorTick.bCanEverTick = true;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	SetRootComponent(Body);
	Body->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	CameraArm->SetupAttachment(Body);
	CameraArm->TargetArmLength = 700.f;          // 车后 7m
	CameraArm->SetRelativeLocation(FVector(0.f, 0.f, 250.f));
	CameraArm->SetRelativeRotation(FRotator(-15.f, 0.f, 0.f));
	CameraArm->bDoCollisionTest = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraArm);

	// 占位车身盒子（4.5×2×1.2m）。换真车模型只换 mesh。
	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		Body->SetStaticMesh(Cube);
		Body->SetWorldScale3D(FVector(4.5f, 2.0f, 1.2f));
	}
}

void ASGDrivableCar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// legacy 轴/动作（见 DefaultInput.ini）：CarThrottle(W/S) 油门、CarSteer(A/D) 转向、
	// CarExit(E) 下车。车被 possess 后走这里的绑定，与角色输入互不干扰。
	PlayerInputComponent->BindAxis(TEXT("CarThrottle"), this, &ASGDrivableCar::Throttle);
	PlayerInputComponent->BindAxis(TEXT("CarSteer"), this, &ASGDrivableCar::Steer);
	PlayerInputComponent->BindAction(TEXT("CarExit"), IE_Pressed, this, &ASGDrivableCar::ExitCar);
}

void ASGDrivableCar::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 油门：朝目标速度逼近；松油门自然减速。
	const float Target = ThrottleInput * MaxSpeed;
	CurrentSpeed = FMath::FInterpTo(CurrentSpeed, Target, DeltaSeconds, Accel / FMath::Max(MaxSpeed, 1.f));
	if (FMath::IsNearlyZero(ThrottleInput))
	{
		CurrentSpeed = FMath::FInterpTo(CurrentSpeed, 0.f, DeltaSeconds, 1.5f); // 阻力
	}

	// 前进
	const FVector Fwd = GetActorForwardVector();
	AddActorWorldOffset(Fwd * CurrentSpeed * DeltaSeconds, /*bSweep=*/true);

	// 转向：仅在有速度时（像真车），转角与速度方向一致
	if (!FMath::IsNearlyZero(CurrentSpeed) && !FMath::IsNearlyZero(SteerInput))
	{
		const float Dir = (CurrentSpeed >= 0.f) ? 1.f : -1.f;
		const float Yaw = SteerInput * TurnRate * DeltaSeconds * Dir
			* FMath::Clamp(FMath::Abs(CurrentSpeed) / MaxSpeed, 0.2f, 1.f);
		AddActorWorldRotation(FRotator(0.f, Yaw, 0.f));
	}

	ThrottleInput = 0.f;
	SteerInput = 0.f;
}

void ASGDrivableCar::Throttle(float Value)
{
	ThrottleInput = Value;
}

void ASGDrivableCar::Steer(float Value)
{
	SteerInput = Value;
}

void ASGDrivableCar::OnInteract_Implementation(AActor* Interactor)
{
	// 玩家走近按 E 上车：记住司机角色，controller possess 这辆车。
	APawn* AsPawn = Cast<APawn>(Interactor);
	if (!AsPawn) { return; }
	AController* Ctrl = AsPawn->GetController();
	APlayerController* PC = Cast<APlayerController>(Ctrl);
	if (!PC) { return; }

	DriverPawn = AsPawn;
	PC->Possess(this);

	// 上车音（E 块）。引擎循环音待真音频资产，用 AudioComponent 接入。
	if (UGameInstance* GI = GetGameInstance())
	{
		if (USGAudioSubsystem* Audio = GI->GetSubsystem<USGAudioSubsystem>())
		{
			Audio->PlayCue2D(ESGSound::CarEnter);
		}
	}
}

FText ASGDrivableCar::GetInteractionPrompt_Implementation() const
{
	return FText::FromString(TEXT("[E] 上车"));
}

void ASGDrivableCar::ExitCar()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC || !DriverPawn) { return; }

	// 把司机角色挪到车旁，possess 回去。
	const FVector Side = GetActorLocation() + GetActorRightVector() * 250.f + FVector(0, 0, 50.f);
	DriverPawn->SetActorLocation(Side);
	PC->Possess(DriverPawn);
	DriverPawn = nullptr;
	CurrentSpeed = 0.f;
}
