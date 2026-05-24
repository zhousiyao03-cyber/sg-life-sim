#include "World/SGSimpleMover.h"
#include "Components/StaticMeshComponent.h"
#include "World/SGTrafficLight.h"
#include "Kismet/GameplayStatics.h"

ASGSimpleMover::ASGSimpleMover()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 占位移动体不参与物理碰撞，避免推开玩家
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

bool ASGSimpleMover::ShouldStop() const
{
	const FVector MyLoc = GetActorLocation();

	// 1) 前车避让：前方一个车身距离内有别的移动体，就停（不穿车）。
	{
		TArray<AActor*> Movers;
		UGameplayStatics::GetAllActorsOfClass(this, ASGSimpleMover::StaticClass(), Movers);
		for (const AActor* Other : Movers)
		{
			if (Other == this) { continue; }
			const FVector ToOther = Other->GetActorLocation() - MyLoc;
			const float Forward = FVector::DotProduct(ToOther, MoveDir);
			// 在正前方（点积 > 0）且很近：车距约 600cm 内、横向偏移小。
			if (Forward > 0.f && Forward < 600.f)
			{
				const float Lateral = (ToOther - MoveDir * Forward).Size2D();
				if (Lateral < 220.f) { return true; }
			}
		}
	}

	// 2) 红灯：前方近处有处于停车相位的信号灯就停。
	{
		TArray<AActor*> Lights;
		UGameplayStatics::GetAllActorsOfClass(this, ASGTrafficLight::StaticClass(), Lights);
		for (const AActor* L : Lights)
		{
			const ASGTrafficLight* Light = Cast<ASGTrafficLight>(L);
			if (!Light || !Light->ShouldVehiclesStop()) { continue; }
			const FVector ToLight = Light->GetActorLocation() - MyLoc;
			const float Forward = FVector::DotProduct(ToLight, MoveDir);
			if (Forward > 0.f && Forward < 700.f)
			{
				const float Lateral = (ToLight - MoveDir * Forward).Size2D();
				if (Lateral < 500.f) { return true; } // 路口范围内迎着红灯
			}
		}
	}

	return false;
}

void ASGSimpleMover::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 车辆遇前车/红灯就停（不推进）；行人照常走。
	if (bIsVehicle && ShouldStop())
	{
		return;
	}

	Traveled += MoveSpeed * DeltaSeconds;
	if (Traveled >= Loop)
	{
		Traveled -= Loop; // wrap 回起点附近，无缝循环
	}
	SetActorLocation(StartLocation + MoveDir * Traveled);
}
