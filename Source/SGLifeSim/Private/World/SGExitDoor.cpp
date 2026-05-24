#include "World/SGExitDoor.h"
#include "World/LocationManagerSubsystem.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/GameInstance.h"

ASGExitDoor::ASGExitDoor()
{
	PrimaryActorTick.bCanEverTick = false;

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	RootComponent = DoorMesh;

	InteractionRange = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionRange"));
	InteractionRange->SetupAttachment(RootComponent);
	InteractionRange->SetSphereRadius(220.f);
	InteractionRange->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	// 占位门板：薄盒子。资产到位后换真门 mesh，逻辑不动。
	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		DoorMesh->SetStaticMesh(Cube);
		DoorMesh->SetWorldScale3D(FVector(0.15f, 1.2f, 2.2f)); // 门板：薄、1.2m 宽、2.2m 高
	}
}

void ASGExitDoor::OnInteract_Implementation(AActor* /*Interactor*/)
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (USGLocationManagerSubsystem* Loc = GI->GetSubsystem<USGLocationManagerSubsystem>())
		{
			Loc->ReturnToCity(); // OpenLevel L_City + 标记回程传送回原楼门口
		}
	}
}

FText ASGExitDoor::GetInteractionPrompt_Implementation() const
{
	return FText::FromString(TEXT("[E] 出门"));
}
