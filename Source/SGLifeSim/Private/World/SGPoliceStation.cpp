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

	// 换皮：用赛道素材的控制塔楼当警察局外观。SM_ControlHouse_B 自带真实米级尺寸，
	// 故 scale 设近 1（不再像 Cube 那样靠 6 倍放大）。★比例待 PIE 校准——
	// ControlHouse 实际包围盒未知，可能需整体调大/调小。
	if (UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/RacingTrack/Mesh/SM_ControlHouse_B.SM_ControlHouse_B")))
	{
		Building->SetStaticMesh(Mesh);
		Building->SetWorldScale3D(FVector(2.f, 2.f, 2.f)); // 保守放大，待校准
	}
	// 蓝色染色作为"这是警察局"的可视标识，保留到美术给真招牌/配色前。
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
