#include "World/SGBuildingEntrance.h"
#include "World/LocationRegistry.h"
#include "World/LocationManagerSubsystem.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/GameInstance.h"

ASGBuildingEntrance::ASGBuildingEntrance()
{
	PrimaryActorTick.bCanEverTick = false;

	BuildingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuildingMesh"));
	RootComponent = BuildingMesh;

	InteractionRange = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionRange"));
	InteractionRange->SetupAttachment(RootComponent);
	InteractionRange->SetSphereRadius(250.f); // 门口稍大些，好走近
	InteractionRange->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void ASGBuildingEntrance::OnInteract_Implementation(AActor* Interactor)
{
	if (Location == ELocation::None)
	{
		return;
	}
	if (UGameInstance* GI = GetGameInstance())
	{
		if (USGLocationManagerSubsystem* Mgr = GI->GetSubsystem<USGLocationManagerSubsystem>())
		{
			Mgr->EnterLocation(Location); // 记城市坐标 + OpenLevel 室内
		}
	}
}

FText ASGBuildingEntrance::GetInteractionPrompt_Implementation() const
{
	const FLocationDef Def = FLocationRegistry::GetLocationDef(Location);
	return FText::FromString(FString::Printf(TEXT("[E] 进入%s"), *Def.DisplayName.ToString()));
}

void ASGBuildingEntrance::ConfigureEntrance(ELocation InLocation)
{
	Location = InLocation;

	// 占位外观：引擎自带 Cube 拉成一栋楼。资产到位后换真建筑 mesh，逻辑不动。
	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		if (BuildingMesh)
		{
			BuildingMesh->SetStaticMesh(Cube);
			BuildingMesh->SetWorldScale3D(FVector(4.f, 4.f, 8.f)); // 一栋小楼
		}
	}
}
