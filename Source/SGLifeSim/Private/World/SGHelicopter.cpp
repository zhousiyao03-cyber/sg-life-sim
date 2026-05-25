#include "World/SGHelicopter.h"

#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/GameInstance.h"
#include "Systems/SGAudioSubsystem.h"

ASGHelicopter::ASGHelicopter()
{
	PrimaryActorTick.bCanEverTick = true;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	SetRootComponent(Body);
	Body->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	Rotor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rotor"));
	Rotor->SetupAttachment(Body);
	Rotor->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	CameraArm->SetupAttachment(Body);
	CameraArm->TargetArmLength = 900.f;
	CameraArm->SetRelativeLocation(FVector(0.f, 0.f, 300.f));
	CameraArm->SetRelativeRotation(FRotator(-18.f, 0.f, 0.f));
	CameraArm->bDoCollisionTest = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraArm);

	// 占位机身（5×2×2m）+ 顶部旋翼盘（扁平大盘）。换真模型只换 mesh。
	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		Body->SetStaticMesh(Cube);
		Body->SetWorldScale3D(FVector(5.f, 2.f, 2.f));
	}
	if (UStaticMesh* Cyl = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")))
	{
		Rotor->SetStaticMesh(Cyl);
		Rotor->SetRelativeScale3D(FVector(2.4f, 2.4f, 0.04f)); // 扁平大盘
		Rotor->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	}
}

void ASGHelicopter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 复用 legacy 轴：CarThrottle(W/S) 前后、CarSteer(A/D) 偏航、HeliUp(空格/Ctrl) 升降、CarExit(E) 下机。
	PlayerInputComponent->BindAxis(TEXT("CarThrottle"), this, &ASGHelicopter::Throttle);
	PlayerInputComponent->BindAxis(TEXT("CarSteer"), this, &ASGHelicopter::Steer);
	PlayerInputComponent->BindAxis(TEXT("HeliUp"), this, &ASGHelicopter::Lift);
	PlayerInputComponent->BindAction(TEXT("CarExit"), IE_Pressed, this, &ASGHelicopter::ExitHeli);
}

void ASGHelicopter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 旋翼自转（占位演出，始终在转）。
	if (Rotor)
	{
		Rotor->AddLocalRotation(FRotator(0.f, RotorSpinRate * DeltaSeconds, 0.f));
	}

	// 前进：朝目标速度逼近，松杆减速。
	const float Target = ThrottleInput * MaxSpeed;
	CurrentSpeed = FMath::FInterpTo(CurrentSpeed, Target, DeltaSeconds, Accel / FMath::Max(MaxSpeed, 1.f));
	if (FMath::IsNearlyZero(ThrottleInput))
	{
		CurrentSpeed = FMath::FInterpTo(CurrentSpeed, 0.f, DeltaSeconds, 1.2f);
	}

	// 水平前进（沿机头水平方向）+ 垂直升降。
	FVector Fwd = GetActorForwardVector();
	Fwd.Z = 0.f;
	Fwd = Fwd.GetSafeNormal();
	FVector Delta = Fwd * CurrentSpeed * DeltaSeconds;
	Delta.Z += LiftInput * LiftSpeed * DeltaSeconds;
	AddActorWorldOffset(Delta, /*bSweep=*/true);

	// 偏航转向（直升机原地也能转）。
	if (!FMath::IsNearlyZero(SteerInput))
	{
		AddActorWorldRotation(FRotator(0.f, SteerInput * TurnRate * DeltaSeconds, 0.f));
	}

	ThrottleInput = 0.f;
	SteerInput = 0.f;
	LiftInput = 0.f;
}

void ASGHelicopter::Throttle(float Value) { ThrottleInput = Value; }
void ASGHelicopter::Steer(float Value) { SteerInput = Value; }
void ASGHelicopter::Lift(float Value) { LiftInput = Value; }

void ASGHelicopter::OnInteract_Implementation(AActor* Interactor)
{
	APawn* AsPawn = Cast<APawn>(Interactor);
	if (!AsPawn) { return; }
	APlayerController* PC = Cast<APlayerController>(AsPawn->GetController());
	if (!PC) { return; }

	DriverPawn = AsPawn;
	PC->Possess(this);

	if (UGameInstance* GI = GetGameInstance())
	{
		if (USGAudioSubsystem* Audio = GI->GetSubsystem<USGAudioSubsystem>())
		{
			Audio->PlayCue2D(ESGSound::CarEnter);
		}
	}
}

FText ASGHelicopter::GetInteractionPrompt_Implementation() const
{
	return FText::FromString(TEXT("[E] 登机（空格升 / Ctrl 降）"));
}

void ASGHelicopter::ExitHeli()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC || !DriverPawn) { return; }

	// 把玩家挪到机身旁，possess 回去。保持机身当前高度（别硬瞬到 Z=100，那会无视地形高度
	// 把空中下机的玩家直接塞进地里）——出机后由 CharacterMovement 重力自然落地。带 sweep 防嵌机身。
	const FVector Side = GetActorLocation() + GetActorRightVector() * 400.f;
	DriverPawn->SetActorLocation(Side, /*bSweep=*/true);
	PC->Possess(DriverPawn);
	DriverPawn = nullptr;
	CurrentSpeed = 0.f;
}
