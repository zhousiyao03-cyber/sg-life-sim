#include "World/SGPoliceStation.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Engine/GameInstance.h"
#include "Systems/WantedSubsystem.h"
#include "Systems/PlayerVitalsSubsystem.h"
#include "Systems/EconomySubsystem.h"
#include "Systems/EconomyTypes.h"

ASGPoliceStation::ASGPoliceStation()
{
	Building = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Building"));
	SetRootComponent(Building);
	Building->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		Building->SetStaticMesh(Cube);
		Building->SetWorldScale3D(FVector(6.f, 6.f, 6.f));
	}
	if (UMaterialInterface* M = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/MI_Car2.MI_Car2")))
	{
		Building->SetMaterial(0, M); // 蓝色 = 警察局
	}
}

void ASGPoliceStation::OnInteract_Implementation(AActor* Interactor)
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) { return; }

	UWantedSubsystem* Wanted = GI->GetSubsystem<UWantedSubsystem>();
	UPlayerVitalsSubsystem* Vitals = GI->GetSubsystem<UPlayerVitalsSubsystem>();
	UEconomySubsystem* Econ = GI->GetSubsystem<UEconomySubsystem>();

	// 有通缉：缴保释金销案（钱不够则不办）。
	if (Wanted && Wanted->GetStars() > 0)
	{
		if (Econ && Econ->TryWithdraw(ECurrencyAccount::Cash, BailCents, TEXT("Bail")))
		{
			Wanted->ClearWanted();
		}
	}

	// 治疗：回满战斗血量（医务室）。
	if (Vitals)
	{
		Vitals->SetHealth(100);
	}
}

FText ASGPoliceStation::GetInteractionPrompt_Implementation() const
{
	return FText::FromString(TEXT("[E] 警察局：缴保释金销案 / 治疗"));
}
