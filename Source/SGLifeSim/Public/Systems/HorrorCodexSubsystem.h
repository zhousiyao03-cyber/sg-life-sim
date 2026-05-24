#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Systems/HorrorCodexSystem.h"
#include "Systems/HorrorEventTypes.h"
#include "HorrorCodexSubsystem.generated.h"

/** 图鉴新解锁一条传说时广播（事件类型 + 标题）。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHorrorCodexEntryUnlocked, EHorrorEvent, Event, FText, Title);

/** 图鉴里的一条（供 UI 列表）。 */
USTRUCT(BlueprintType)
struct FHorrorCodexEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Horror")
	EHorrorEvent Event = EHorrorEvent::None;

	/** 是否已亲历。未亲历则 UI 显示「？？？」。 */
	UPROPERTY(BlueprintReadOnly, Category = "Horror")
	bool bDiscovered = false;

	/** 已亲历才填阴森文案；未亲历为空。 */
	UPROPERTY(BlueprintReadOnly, Category = "Horror")
	FText Title;
};

/**
 * 恐怖图鉴子系统（Plan 21）。FHorrorCodexSystem 的 UE 薄壳。
 *
 * 订阅 UHorrorEventSubsystem::OnHorrorEventTyped，把亲历过的都市传说记进图鉴。
 * 首次发现 → 解锁 FirstUrbanLegend 成就；集齐 → CompleteHorrorCodex。
 * 提供 GetEntries（已/未发现 + 文案）供 UI 列出。进存档（bitmask）。
 */
UCLASS()
class SGLIFESIM_API UHorrorCodexSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 图鉴新解锁一条时广播。 */
	UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Horror")
	FOnHorrorCodexEntryUnlocked OnEntryUnlocked;

	/** 已发现条目数。 */
	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Horror")
	int32 GetDiscoveredCount() const { return Codex.CountDiscovered(); }

	/** 图鉴总条目数。 */
	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Horror")
	int32 GetTotalCount() const { return FHorrorCodexSystem::TotalCollectable(); }

	/** 是否已亲历某事件。 */
	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Horror")
	bool HasDiscovered(EHorrorEvent Event) const { return Codex.HasEncountered(Event); }

	/** 「N / M」进度文案，供 HUD/菜单。 */
	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Horror")
	FText GetProgressText() const;

	/** 取整本图鉴（含未发现的占位条目），供 UI 列表。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Horror")
	TArray<FHorrorCodexEntry> GetEntries() const;

	/** 直接记录一条（测试 / 剧情直触发用）。首次发现返回 true。 */
	bool RecordEncounter(EHorrorEvent Event);

	/** 存档接口。 */
	int64 GetMaskForSave() const { return Codex.GetMask(); }
	void RestoreFromSave(int64 Mask) { Codex.RestoreMask(Mask); }

private:
	UFUNCTION()
	void HandleHorrorEventTyped(EHorrorEvent Event);

	FHorrorCodexSystem Codex;
};
