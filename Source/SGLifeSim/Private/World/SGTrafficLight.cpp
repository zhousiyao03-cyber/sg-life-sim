#include "World/SGTrafficLight.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"

ASGTrafficLight::ASGTrafficLight()
{
	PrimaryActorTick.bCanEverTick = true;

	Pole = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pole"));
	SetRootComponent(Pole);
	Pole->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Head = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Head"));
	Head->SetupAttachment(Pole);
	Head->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (UStaticMesh* Cyl = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")))
	{
		Pole->SetStaticMesh(Cyl);
		Pole->SetWorldScale3D(FVector(0.12f, 0.12f, 3.0f)); // 细高灯柱（约 3m）
	}
	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		Head->SetStaticMesh(Cube);
		Head->SetRelativeScale3D(FVector(0.4f, 0.4f, 1.0f));
		Head->SetRelativeLocation(FVector(0.f, 0.f, 160.f)); // 顶部灯头
	}
}

void ASGTrafficLight::BeginPlay()
{
	Super::BeginPlay();
	ApplyHeadColor();
}

void ASGTrafficLight::ConfigureLight(ETrafficPhase InitialPhase, float GreenSeconds, float YellowSeconds, float RedSeconds)
{
	GreenTime = GreenSeconds;
	YellowTime = YellowSeconds;
	RedTime = RedSeconds;
	EnterPhase(InitialPhase);
}

void ASGTrafficLight::EnterPhase(ETrafficPhase NewPhase)
{
	Phase = NewPhase;
	switch (Phase)
	{
	case ETrafficPhase::Green:  PhaseTimer = GreenTime;  break;
	case ETrafficPhase::Yellow: PhaseTimer = YellowTime; break;
	case ETrafficPhase::Red:    PhaseTimer = RedTime;    break;
	}
	ApplyHeadColor();
}

void ASGTrafficLight::ApplyHeadColor()
{
	// 用城市配色材质凑红/黄/绿（黄用路灯黄占位）。
	const TCHAR* Mat =
		(Phase == ETrafficPhase::Green)  ? TEXT("/Game/Materials/MI_Ground.MI_Ground") :  // 草绿当绿灯
		(Phase == ETrafficPhase::Yellow) ? TEXT("/Game/Materials/MI_Lamp.MI_Lamp") :       // 灯黄当黄灯
		                                   TEXT("/Game/Materials/MI_Car.MI_Car");          // 红当红灯
	if (UMaterialInterface* M = LoadObject<UMaterialInterface>(nullptr, Mat))
	{
		if (Head) { Head->SetMaterial(0, M); }
	}
}

void ASGTrafficLight::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	PhaseTimer -= DeltaSeconds;
	if (PhaseTimer > 0.f) { return; }

	// 相位轮转：绿→黄→红→绿。
	switch (Phase)
	{
	case ETrafficPhase::Green:  EnterPhase(ETrafficPhase::Yellow); break;
	case ETrafficPhase::Yellow: EnterPhase(ETrafficPhase::Red);    break;
	case ETrafficPhase::Red:    EnterPhase(ETrafficPhase::Green);  break;
	}
}
