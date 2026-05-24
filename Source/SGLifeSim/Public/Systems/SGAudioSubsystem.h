#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SGAudioSubsystem.generated.h"

class USoundBase;

/** 游戏音效种类（E 块 GTA）。一个语义入口对应一个声音资产引用。 */
UENUM(BlueprintType)
enum class ESGSound : uint8
{
	Footstep    UMETA(DisplayName = "脚步"),
	Gunshot     UMETA(DisplayName = "枪声"),
	Reload      UMETA(DisplayName = "换弹"),
	Punch       UMETA(DisplayName = "出拳"),
	NpcHit      UMETA(DisplayName = "命中NPC"),
	PlayerHurt  UMETA(DisplayName = "玩家受伤"),
	PlayerDeath UMETA(DisplayName = "玩家死亡"),
	CarEnter    UMETA(DisplayName = "上车"),
	CarEngine   UMETA(DisplayName = "引擎"),
	UIClick     UMETA(DisplayName = "界面点击"),
	Horror      UMETA(DisplayName = "恐怖刺激"),
};

/**
 * 音效系统壳（E 块 GTA）。把「播放某类音效」收口成一个语义入口 PlayCue，
 * 各处（开火/出拳/上车/受伤…）只管报「发生了什么」，不关心具体音频资产。
 *
 * 关键现实约束：项目暂无自制音频素材。所以每种音效的 USoundBase 引用默认空，
 * 空时回退到引擎自带合成音 1kSineTonePing 当占位「哔」声（bUsePlaceholderBeeps），
 * 保证运行时「有声音反馈」可验证。美术/音频接入时只需 SetCue 填真音并关占位。
 */
UCLASS()
class SGLIFESIM_API USGAudioSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** 播放 2D 音效（UI / 玩家自身动作，无空间感）。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Audio")
	void PlayCue2D(ESGSound Sound, float VolumeMultiplier = 1.f, float PitchMultiplier = 1.f);

	/** 在世界某处播放音效（带空间感，如远处枪声/引擎）。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Audio")
	void PlayCueAtLocation(ESGSound Sound, const FVector& Location, float VolumeMultiplier = 1.f, float PitchMultiplier = 1.f);

	/** 注入某类音效的真实音频资产（换皮时调）。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Audio")
	void SetCue(ESGSound Sound, USoundBase* Cue);

	/** 占位「哔」声开关（默认开；填了真音后可关）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SGLifeSim|Audio")
	bool bUsePlaceholderBeeps = true;

private:
	/** 解析某类音效要播的 USoundBase：优先用 SetCue 注入的真音，否则占位哔。 */
	USoundBase* ResolveSound(ESGSound Sound) const;

	/** 不同音效用不同音高的占位哔，至少听感上能区分（枪声高、脚步低…）。 */
	float PlaceholderPitch(ESGSound Sound) const;

	/** 各类音效的真实资产引用（默认空 → 走占位哔）。 */
	UPROPERTY(Transient)
	TMap<ESGSound, TObjectPtr<USoundBase>> Cues;

	/** 缓存的占位哔声资产（引擎自带 1kSineTonePing）。 */
	UPROPERTY(Transient)
	TObjectPtr<USoundBase> PlaceholderBeep;
};
