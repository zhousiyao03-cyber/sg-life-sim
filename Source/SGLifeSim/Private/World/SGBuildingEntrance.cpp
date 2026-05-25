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

	// 换皮：按地点类型选赛道素材里的建筑 mesh，让城市有辨识度（公司用玻璃写字楼等）。
	// 这些 mesh 自带真实米级尺寸，scale 设近 1。★比例/朝向待 PIE 校准。
	const TCHAR* MeshPath;
	switch (Location)
	{
	case ELocation::Office:
		MeshPath = TEXT("/Game/RacingTrack/Mesh/SM_Build_GlassBlock.SM_Build_GlassBlock"); // 玻璃幕墙写字楼
		break;
	case ELocation::Mall:
	case ELocation::MRT:
		MeshPath = TEXT("/Game/RacingTrack/Mesh/SM_ControlHouse_B.SM_ControlHouse_B"); // 大些的公共建筑
		break;
	default: // Rental / Hawker / Corridor
		MeshPath = TEXT("/Game/RacingTrack/Mesh/SM_ControlHouse_A.SM_ControlHouse_A"); // 小楼
		break;
	}

	if (UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, MeshPath))
	{
		if (BuildingMesh)
		{
			BuildingMesh->SetStaticMesh(Mesh);
			BuildingMesh->SetWorldScale3D(FVector(2.f, 2.f, 2.f)); // 保守放大，待校准
		}
	}
}
