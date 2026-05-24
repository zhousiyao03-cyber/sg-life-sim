#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveGameSubsystem.generated.h"

class USGSaveGame;

/**
 * 存档子系统。Plan 2 Task 7。
 *
 * 协调五大系统的存读档：采集各 GameInstance 子系统状态 → USGSaveGame →
 * UGameplayStatics::SaveGameToSlot；读档反向回灌。是唯一知道「全局状态长什么样」
 * 的地方，单个系统不互相依赖。
 */
UCLASS()
class SGLIFESIM_API USaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 默认存档槽名。 */
	static const FString DefaultSlot;

	/** 把当前所有系统状态写入指定槽。成功返回 true。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Save")
	bool SaveToSlot(const FString& SlotName);

	/** 从指定槽读档并回灌到各系统。成功返回 true。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Save")
	bool LoadFromSlot(const FString& SlotName);

	/** 指定槽是否存在存档。 */
	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Save")
	bool DoesSaveExist(const FString& SlotName) const;

private:
	/** 从各子系统采集状态填进 Save 对象。 */
	void GatherInto(USGSaveGame& Save) const;

	/** 把 Save 对象的状态回灌到各子系统。 */
	void ApplyFrom(const USGSaveGame& Save);
};
