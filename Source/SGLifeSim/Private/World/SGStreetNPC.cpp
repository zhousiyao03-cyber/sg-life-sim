#include "World/SGStreetNPC.h"
#include "Systems/WantedSubsystem.h"
#include "Systems/PlayerVitalsSubsystem.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

ASGStreetNPC::ASGStreetNPC()
{
	PrimaryActorTick.bCanEverTick = true;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	SetRootComponent(Body);
	Body->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	Head = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Head"));
	Head->SetupAttachment(Body);

	if (UStaticMesh* Cyl = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")))
	{
		Body->SetStaticMesh(Cyl);
		Body->SetWorldScale3D(FVector(0.35f, 0.35f, 1.8f));
	}
	if (UStaticMesh* Sph = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")))
	{
		Head->SetStaticMesh(Sph);
		Head->SetRelativeScale3D(FVector(1.3f, 1.3f, 0.28f)); // 相对身体（身体已 1.8 高）
		Head->SetRelativeLocation(FVector(0.f, 0.f, 95.f));
	}
}

void ASGStreetNPC::BeginPlay()
{
	Super::BeginPlay();
	// 不同类型给不同占位色（用城市配色材质）。
	const TCHAR* Mat =
		(Kind == EStreetNpcKind::Police)   ? TEXT("/Game/Materials/MI_Car2.MI_Car2") :   // 蓝=警察
		(Kind == EStreetNpcKind::Gangster) ? TEXT("/Game/Materials/MI_Car.MI_Car") :     // 红=帮派
		                                     TEXT("/Game/Materials/MI_PedBody.MI_PedBody");
	if (UMaterialInterface* M = LoadObject<UMaterialInterface>(nullptr, Mat))
	{
		if (Body) { Body->SetMaterial(0, M); }
	}
}

void ASGStreetNPC::ConfigureKind(EStreetNpcKind InKind)
{
	Kind = InKind;
	Health = (Kind == EStreetNpcKind::Police) ? 150 : 100;
}

void ASGStreetNPC::TakeMeleeHit(int32 Damage)
{
	if (bDead) { return; }
	Health -= Damage;

	// 被打到 → 涨通缉（袭击路人/警察都犯事）。
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UWantedSubsystem* W = GI->GetSubsystem<UWantedSubsystem>())
		{
			W->AddHeat(Kind == EStreetNpcKind::Police ? 60 : 25);
		}
	}

	if (Health <= 0)
	{
		bDead = true;
		// 占位「死亡」：倒地 + 隐藏碰撞。
		SetActorRotation(FRotator(90.f, GetActorRotation().Yaw, 0.f));
		if (Body) { Body->SetCollisionEnabled(ECollisionEnabled::NoCollision); }
	}
}

AActor* ASGStreetNPC::FindPlayer() const
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		return PC->GetPawn();
	}
	return nullptr;
}

void ASGStreetNPC::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bDead) { return; }

	// 警察：玩家有通缉就直线追；靠近就逮捕（清通缉）。帮派：直接敌对靠近。
	const bool bChaser = (Kind == EStreetNpcKind::Police || Kind == EStreetNpcKind::Gangster);
	if (!bChaser) { return; }

	UGameInstance* GI = GetGameInstance();
	UWantedSubsystem* W = GI ? GI->GetSubsystem<UWantedSubsystem>() : nullptr;

	bool bShouldChase = (Kind == EStreetNpcKind::Gangster); // 帮派总敌对
	if (Kind == EStreetNpcKind::Police && W && W->GetStars() > 0)
	{
		bShouldChase = true;
	}
	if (!bShouldChase) { return; }

	AActor* Player = FindPlayer();
	if (!Player) { return; }

	const FVector ToP = Player->GetActorLocation() - GetActorLocation();
	const float Dist = ToP.Size2D();

	AttackCooldown -= DeltaSeconds;

	// 靠得很近：警察逮捕（清通缉），帮派挥拳扣玩家血（B 块打斗）。
	if (Dist < 180.f)
	{
		// 面朝玩家。
		FVector Face = ToP; Face.Z = 0.f; Face = Face.GetSafeNormal();
		if (!Face.IsNearlyZero()) { SetActorRotation(Face.Rotation()); }

		if (Kind == EStreetNpcKind::Police && W)
		{
			W->ClearWanted();
		}
		else if (Kind == EStreetNpcKind::Gangster && AttackCooldown <= 0.f)
		{
			AttackCooldown = 1.2f;
			if (UGameInstance* GI2 = GetGameInstance())
			{
				if (UPlayerVitalsSubsystem* Vit = GI2->GetSubsystem<UPlayerVitalsSubsystem>())
				{
					Vit->ApplyDamage(12);
				}
			}
		}
		PathPoints.Reset();
		return;
	}

	// 沿 NavMesh 寻路绕开建筑朝玩家走。
	ChaseTowards(Player->GetActorLocation(), DeltaSeconds);
}

void ASGStreetNPC::ChaseTowards(const FVector& TargetLocation, float DeltaSeconds)
{
	// 目标在动，周期性重算路径（每 0.4s，或路径已走完）。
	RepathCooldown -= DeltaSeconds;
	const bool bNeedRepath = (RepathCooldown <= 0.f) || !PathPoints.IsValidIndex(PathIndex);
	if (bNeedRepath)
	{
		RepathCooldown = 0.4f;
		PathPoints.Reset();
		PathIndex = 0;

		if (UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
		{
			if (UNavigationPath* NavPath = Nav->FindPathToLocationSynchronously(GetWorld(), GetActorLocation(), TargetLocation))
			{
				if (NavPath->IsValid() && NavPath->PathPoints.Num() > 1)
				{
					PathPoints = NavPath->PathPoints;
					// 跳过起点（就是自己当前位置），从下一个拐点开始走。
					PathIndex = 1;
				}
			}
		}
	}

	// 选当前要去的路径点；无有效路径则回退直线朝目标（保底不卡死）。
	FVector NextPoint = TargetLocation;
	if (PathPoints.IsValidIndex(PathIndex))
	{
		NextPoint = PathPoints[PathIndex];
		// 到达当前拐点就推进到下一个。
		if (FVector::DistSquared2D(GetActorLocation(), NextPoint) < FMath::Square(60.f))
		{
			++PathIndex;
			if (PathPoints.IsValidIndex(PathIndex))
			{
				NextPoint = PathPoints[PathIndex];
			}
		}
	}

	FVector Dir = NextPoint - GetActorLocation();
	Dir.Z = 0.f;
	Dir = Dir.GetSafeNormal();
	if (Dir.IsNearlyZero()) { return; }

	AddActorWorldOffset(Dir * ChaseSpeed * DeltaSeconds, /*bSweep=*/true);
	SetActorRotation(Dir.Rotation());
}
