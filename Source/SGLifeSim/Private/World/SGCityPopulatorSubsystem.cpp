#include "World/SGCityPopulatorSubsystem.h"
#include "World/LocationRegistry.h"
#include "World/LocationTypes.h"
#include "World/SGBuildingEntrance.h"
#include "World/SGSimpleMover.h"
#include "World/SGTrafficLight.h"
#include "World/SGHelicopter.h"
#include "World/SGDrivableCar.h"
#include "World/SGPoliceStation.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"

bool USGCityPopulatorSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

TArray<FDecorBuildingSpec> USGCityPopulatorSubsystem::BuildDecorLayout(
	const TArray<FVector>& EntranceLocations, float HalfExtent, float Spacing, float ClearRadius)
{
	TArray<FDecorBuildingSpec> Out;
	if (Spacing <= 0.f)
	{
		return Out;
	}

	const float ClearSq = ClearRadius * ClearRadius;

	// 在 [-HalfExtent, HalfExtent] 格点铺楼。确定性：纯按坐标 hash 定高度，无随机状态。
	for (float X = -HalfExtent; X <= HalfExtent; X += Spacing)
	{
		for (float Y = -HalfExtent; Y <= HalfExtent; Y += Spacing)
		{
			const FVector Cell(X, Y, 0.f);

			// 跳过靠近任何入口建筑的格子（给可进建筑让位 + 留出门前空地）。
			bool bTooClose = false;
			for (const FVector& E : EntranceLocations)
			{
				if (FVector::DistSquared2D(Cell, E) < ClearSq) { bTooClose = true; break; }
			}
			if (bTooClose)
			{
				continue;
			}

			// 高度按格坐标确定性变化（错落感），不依赖随机。
			const int32 H = 6 + (FMath::Abs((int32)(X / Spacing) * 7 + (int32)(Y / Spacing) * 3) % 10);

			FDecorBuildingSpec Spec;
			Spec.Location = FVector(X, Y, H * 50.f); // 抬高半个身位让楼立在地面上
			Spec.Scale = FVector(3.f, 3.f, (float)H);
			Out.Add(Spec);
		}
	}
	return Out;
}

void USGCityPopulatorSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// 只在城市枢纽关卡铺城市。
	const FString LevelName = UWorld::RemovePIEPrefix(InWorld.GetMapName());
	if (!LevelName.Contains(FLocationRegistry::GetCityLevelName().ToString()))
	{
		return;
	}

	// 幂等：已铺过（存在任一 BuildingEntrance）则跳过。
	{
		TArray<AActor*> Existing;
		UGameplayStatics::GetAllActorsOfClass(&InWorld, ASGBuildingEntrance::StaticClass(), Existing);
		if (Existing.Num() > 0)
		{
			return;
		}
	}

	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* Plane = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));

	// 地面。
	if (Plane)
	{
		AActor* Ground = InWorld.SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		if (Ground)
		{
			UStaticMeshComponent* GC = NewObject<UStaticMeshComponent>(Ground);
			GC->RegisterComponent();
			Ground->SetRootComponent(GC);
			GC->SetStaticMesh(Plane);
			GC->SetWorldScale3D(FVector(120.f, 120.f, 1.f)); // 大片地面
		}
	}

	// 可进建筑入口（按注册表）。
	TArray<FVector> EntranceLocs;
	for (int32 i = 1; i < (int32)ELocation::Count; ++i)
	{
		const ELocation Loc = (ELocation)i;
		const FLocationDef Def = FLocationRegistry::GetLocationDef(Loc);
		if (Def.LevelName.IsNone())
		{
			continue;
		}

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		ASGBuildingEntrance* Entrance = InWorld.SpawnActor<ASGBuildingEntrance>(
			ASGBuildingEntrance::StaticClass(), Def.CityLocation, FRotator::ZeroRotator, Params);
		if (Entrance)
		{
			Entrance->ConfigureEntrance(Loc);
			EntranceLocs.Add(Def.CityLocation);
		}
	}

	// 装饰楼（不可进，填充城市感）。
	if (Cube)
	{
		const TArray<FDecorBuildingSpec> Decor = BuildDecorLayout(
			EntranceLocs, /*HalfExtent=*/6000.f, /*Spacing=*/900.f, /*ClearRadius=*/700.f);
		for (const FDecorBuildingSpec& Spec : Decor)
		{
			AActor* Bldg = InWorld.SpawnActor<AActor>(AActor::StaticClass(), Spec.Location, FRotator::ZeroRotator);
			if (!Bldg) { continue; }
			UStaticMeshComponent* MC = NewObject<UStaticMeshComponent>(Bldg);
			MC->RegisterComponent();
			Bldg->SetRootComponent(MC);
			MC->SetStaticMesh(Cube);
			MC->SetWorldScale3D(Spec.Scale);
		}
	}

	// 交通系统（F 块）：主干道（X 轴、Y 轴中线）上摆红绿灯路口 + 车流。
	PopulateTraffic(InWorld);
}

void USGCityPopulatorSubsystem::PopulateTraffic(UWorld& InWorld)
{
	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// 几个主干道路口：每个路口摆一对相位相反的红绿灯（南北 vs 东西）。
	const float RoadZ = 10.f;
	const TArray<FVector> Junctions = { FVector(2000, 0, 0), FVector(-2000, 0, 0), FVector(0, 2000, 0), FVector(0, -2000, 0) };
	for (const FVector& J : Junctions)
	{
		// 管东西向车流的灯（放路口侧旁），起始绿。
		if (ASGTrafficLight* L1 = InWorld.SpawnActor<ASGTrafficLight>(ASGTrafficLight::StaticClass(), J + FVector(0, 260, RoadZ), FRotator::ZeroRotator, P))
		{
			L1->ConfigureLight(ETrafficPhase::Green);
		}
		// 管南北向车流的灯，起始红（与上面错开 → 一组绿时另一组红）。
		if (ASGTrafficLight* L2 = InWorld.SpawnActor<ASGTrafficLight>(ASGTrafficLight::StaticClass(), J + FVector(260, 0, RoadZ), FRotator(0, 90, 0), P))
		{
			L2->ConfigureLight(ETrafficPhase::Red);
		}
	}

	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UMaterialInterface* CarMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/MI_Car.MI_Car"));

	// 主干道车流：沿 X 轴（东西）和 Y 轴（南北）各放几辆 vehicle Mover，会避让前车、遇红灯停。
	auto SpawnCar = [&](const FVector& Start, const FVector& Dir)
	{
		ASGSimpleMover* Car = InWorld.SpawnActor<ASGSimpleMover>(ASGSimpleMover::StaticClass(), Start, Dir.Rotation(), P);
		if (!Car) { return; }
		if (Cube && Car->Mesh)
		{
			Car->Mesh->SetStaticMesh(Cube);
			Car->Mesh->SetWorldScale3D(FVector(4.f, 1.8f, 1.1f)); // 占位车身
			Car->Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // 可被射线/检测，不物理推挤
			if (CarMat) { Car->Mesh->SetMaterial(0, CarMat); }
		}
		Car->ConfigureMover(Dir, /*Speed=*/450.f, /*LoopLength=*/12000.f);
		Car->SetIsVehicle(true);
	};

	// 东西向（沿 +X），分布在 Y=0 主干道上、错开起点形成车队。
	for (int32 i = 0; i < 4; ++i)
	{
		SpawnCar(FVector(-6000 + i * 1500.f, -60.f, 90.f), FVector(1, 0, 0));
	}
	// 南北向（沿 +Y），X=0 主干道。
	for (int32 i = 0; i < 4; ++i)
	{
		SpawnCar(FVector(60.f, -6000 + i * 1500.f, 90.f), FVector(0, 1, 0));
	}

	// 直升机（H 块）：停在城市一角的停机坪，走近按 E 登机起飞。
	if (ASGHelicopter* Heli = InWorld.SpawnActor<ASGHelicopter>(
			ASGHelicopter::StaticClass(), FVector(-3500.f, 3500.f, 120.f), FRotator::ZeroRotator, P))
	{
		(void)Heli; // 外观由类内占位，飞行逻辑自带
	}

	// 一辆停着的可上车摩托（占位小号 DrivableCar 变体；真摩托模型待美术换皮）。
	if (ASGDrivableCar* Bike = InWorld.SpawnActor<ASGDrivableCar>(
			ASGDrivableCar::StaticClass(), FVector(800.f, -300.f, 60.f), FRotator::ZeroRotator, P))
	{
		(void)Bike;
	}

	// 警察局 / 帮派地盘地标（H 块）：占位大楼 + 标记色，城市里两个特征地标。
	UStaticMesh* LandmarkCube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	auto SpawnLandmark = [&](const FVector& Loc, const FVector& Scale, const TCHAR* MatPath)
	{
		AActor* A = InWorld.SpawnActor<AActor>(AActor::StaticClass(), Loc, FRotator::ZeroRotator, P);
		if (!A || !LandmarkCube) { return; }
		UStaticMeshComponent* MC = NewObject<UStaticMeshComponent>(A);
		MC->RegisterComponent();
		A->SetRootComponent(MC);
		MC->SetStaticMesh(LandmarkCube);
		MC->SetWorldScale3D(Scale);
		if (UMaterialInterface* M = LoadObject<UMaterialInterface>(nullptr, MatPath))
		{
			MC->SetMaterial(0, M);
		}
	};
	// 警察局：可交互蓝楼（走近 E 缴保释金销案 / 治疗）。
	InWorld.SpawnActor<ASGPoliceStation>(ASGPoliceStation::StaticClass(), FVector(4000.f, -4000.f, 300.f), FRotator::ZeroRotator, P);
	// 帮派地盘：红色大楼（MI_Car 是红）——这一片也是帮派 NPC 聚集处。
	SpawnLandmark(FVector(-4000.f, -4000.f, 300.f), FVector(6.f, 6.f, 6.f), TEXT("/Game/Materials/MI_Car.MI_Car"));
}
