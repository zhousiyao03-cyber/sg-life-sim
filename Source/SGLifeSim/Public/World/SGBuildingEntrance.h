#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactables/InteractableInterface.h"
#include "World/LocationTypes.h"
#include "SGBuildingEntrance.generated.h"

class UStaticMeshComponent;
class USphereComponent;

/**
 * 城市里一栋可进入建筑的门口（开放城市枢纽，2026-05-24）。
 * 实现 IInteractableInterface：玩家走近显「[E] 进入 X」，按 E → LocationManager 切进室内关卡。
 *
 * 与 ASGInteractableNPC 同一套交互机制（E 键 + 范围球），由 PlayerCharacter 的
 * FindNearbyInteractable（改按接口找后）统一发现。占位盒子 mesh，资产到位后换。
 *
 * 见 docs/superpowers/specs/2026-05-24-open-city-hub-design.md。
 */
UCLASS()
class SGLIFESIM_API ASGBuildingEntrance : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	ASGBuildingEntrance();

	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() const override;

	/** 代码生成时设这个入口对应哪个地点（CityPopulator 用）。 */
	void ConfigureEntrance(ELocation InLocation);

	ELocation GetLocation() const { return Location; }

protected:
	/** 建筑外壳（占位：拉伸的盒子）。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Building")
	TObjectPtr<UStaticMeshComponent> BuildingMesh;

	/** 交互范围（玩家进入此球体即可按 E 进入）。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Building")
	TObjectPtr<USphereComponent> InteractionRange;

	/** 这个门口通向哪个地点。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Building")
	ELocation Location = ELocation::None;
};
