#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactables/InteractableInterface.h"
#include "SGPoliceStation.generated.h"

class UStaticMeshComponent;

/**
 * 警察局地标（H 块 GTA）。走近按 E 自首/缴费销案：
 *  - 有通缉：花一笔保释金清通缉（比死亡医院费便宜，给玩家主动消案的正经出口）。
 *  - 顺带治疗：回满战斗血量（局里医务室）。
 *  - 无通缉且满血：提示「这里没你的事」。
 *
 * 占位蓝色大楼（外观由 CityPopulator 设），交互逻辑在此。
 */
UCLASS()
class SGLIFESIM_API ASGPoliceStation : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	ASGPoliceStation();

	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() const override;

	/** 保释金（分）：清通缉的代价，比死亡医院费便宜。 */
	static constexpr int64 BailCents = 20000; // S$200

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Police")
	TObjectPtr<UStaticMeshComponent> Building;
};
