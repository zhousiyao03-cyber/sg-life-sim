#include "World/LocationManagerSubsystem.h"
#include "World/LocationRegistry.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

bool USGLocationManagerSubsystem::EnterLocation(ELocation Location)
{
	const FLocationDef Def = FLocationRegistry::GetLocationDef(Location);
	if (Def.LevelName.IsNone())
	{
		return false;
	}

	UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	if (!World)
	{
		return false;
	}

	// 记下当前城市坐标（回程传送回这栋楼门口）。
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			RememberCityTransform(Pawn->GetActorLocation(), Pawn->GetActorRotation());
		}
	}

	CurrentLocation = Location;
	UGameplayStatics::OpenLevel(World, Def.LevelName);
	return true;
}

void USGLocationManagerSubsystem::ReturnToCity()
{
	CurrentLocation = ELocation::None;
	bPendingReturn = true; // 城市加载后玩家 BeginPlay 取坐标传送回门口

	if (UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr)
	{
		UGameplayStatics::OpenLevel(World, FLocationRegistry::GetCityLevelName());
	}
}

bool USGLocationManagerSubsystem::ConsumePendingReturn(FVector& OutLocation, FRotator& OutRotation)
{
	if (!bPendingReturn)
	{
		return false;
	}
	OutLocation = CityReturnLocation;
	OutRotation = CityReturnRotation;
	bPendingReturn = false; // 只生效一次
	return true;
}

void USGLocationManagerSubsystem::RememberCityTransform(const FVector& Location, const FRotator& Rotation)
{
	CityReturnLocation = Location;
	CityReturnRotation = Rotation;
}
