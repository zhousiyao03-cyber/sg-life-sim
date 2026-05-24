#include "World/SGCityPopulatorSubsystem.h"
#include "World/LocationRegistry.h"
#include "World/LocationTypes.h"
#include "World/SGBuildingEntrance.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
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
}
