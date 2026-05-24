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
		return;
	}

	// 路人挨打但没死：受惊逃跑（G 块）。帮派/警察不逃（继续敌对/执法）。
	if (Kind == EStreetNpcKind::Pedestrian)
	{
		Panic();
	}
}

void ASGStreetNPC::Panic()
{
	const bool bWasCalm = !bAlarmed;
	bAlarmed = true;
	FleeTimer = 6.f; // 受惊后逃 6 秒

	// 目击者举报：第一次受惊涨一笔通缉（路人看到有人动手/掏枪，报警）。
	if (bWasCalm)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UWantedSubsystem* W = GI->GetSubsystem<UWantedSubsystem>())
			{
				W->AddHeat(15);
			}
		}
	}
	PathPoints.Reset();
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

	UGameInstance* GI = GetGameInstance();
	UWantedSubsystem* W = GI ? GI->GetSubsystem<UWantedSubsystem>() : nullptr;

	// 路人（G 块）：感知到危险就受惊逃跑。危险 = 玩家在通缉中（街上出事了）+ 离得近。
	if (Kind == EStreetNpcKind::Pedestrian)
	{
		AActor* PlayerActor = FindPlayer();
		if (PlayerActor)
		{
			const float DistToPlayer = (PlayerActor->GetActorLocation() - GetActorLocation()).Size2D();
			// 玩家有通缉星且在视野近处 → 受惊（目击者举报在 Panic 里）。
			if (!bAlarmed && W && W->GetStars() > 0 && DistToPlayer < 1500.f)
			{
				Panic();
			}

			if (bAlarmed)
			{
				FleeTimer -= DeltaSeconds;
				// 威胁已远离且冷静计时到 → 恢复平静（停下，不再逃）。
				if (FleeTimer <= 0.f && DistToPlayer > 2000.f)
				{
					bAlarmed = false;
				}
				else
				{
					FleeFrom(PlayerActor->GetActorLocation(), DeltaSeconds);
				}
			}
		}
		return;
	}

	// 警察：玩家有通缉就追；靠近逮捕（清通缉）。帮派：直接敌对靠近、贴近挥拳。
	const bool bChaser = (Kind == EStreetNpcKind::Police || Kind == EStreetNpcKind::Gangster);
	if (!bChaser) { return; }

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

void ASGStreetNPC::FleeFrom(const FVector& ThreatLocation, float DeltaSeconds)
{
	// 朝背离威胁的水平方向跑（受惊乱跑，不做精细寻路；bSweep 防穿墙、撞墙自然贴墙滑）。
	FVector Away = GetActorLocation() - ThreatLocation;
	Away.Z = 0.f;
	Away = Away.GetSafeNormal();
	if (Away.IsNearlyZero())
	{
		Away = GetActorForwardVector(); // 重合时随便往前
	}
	AddActorWorldOffset(Away * FleeSpeed * DeltaSeconds, /*bSweep=*/true);
	SetActorRotation(Away.Rotation());
}
