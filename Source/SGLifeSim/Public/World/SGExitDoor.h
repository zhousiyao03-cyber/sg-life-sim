#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactables/InteractableInterface.h"
#include "SGExitDoor.generated.h"

class UStaticMeshComponent;
class USphereComponent;

/**
 * 室内关卡里一扇「出门」（第3块）。BuildingEntrance 的反向版：
 * 放在室内，玩家走近显「[E] 出门」，按 E → LocationManager::ReturnToCity()
 * 回城市枢纽（传送回离开时那栋楼门口）。
 *
 * 比 M 菜单的「出门」更直观——看得见的门、走过去按 E，符合「每个房间有出口」。
 * 与 BuildingEntrance / NPC 同一套交互机制（IInteractableInterface + 范围球）。
 * 占位盒子门，美术后换皮。
 */
UCLASS()
class SGLIFESIM_API ASGExitDoor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	ASGExitDoor();

	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() const override;

protected:
	/** 门外观（占位：薄盒子门板）。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Exit")
	TObjectPtr<UStaticMeshComponent> DoorMesh;

	/** 交互范围。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Exit")
	TObjectPtr<USphereComponent> InteractionRange;
};
