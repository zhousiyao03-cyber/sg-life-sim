#include "World/SGSimpleMover.h"
#include "Components/StaticMeshComponent.h"

ASGSimpleMover::ASGSimpleMover()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 占位移动体不参与碰撞，避免推开玩家
}

void ASGSimpleMover::ConfigureMover(FVector Direction, float Speed, float LoopLength)
{
	StartLocation = GetActorLocation();
	Direction.Z = 0.f;
	MoveDir = Direction.IsNearlyZero() ? FVector::ForwardVector : Direction.GetSafeNormal();
	MoveSpeed = Speed;
	Loop = (LoopLength > 1.f) ? LoopLength : 10000.f;
	Traveled = 0.f;

	// 朝向移动方向（车头/人脸朝前走）。
	SetActorRotation(MoveDir.Rotation());
}

void ASGSimpleMover::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	Traveled += MoveSpeed * DeltaSeconds;
	if (Traveled >= Loop)
	{
		Traveled -= Loop; // wrap 回起点附近，无缝循环
	}
	SetActorLocation(StartLocation + MoveDir * Traveled);
}
