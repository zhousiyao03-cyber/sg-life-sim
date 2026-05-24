#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SGWorldPopulatorSubsystem.generated.h"

/**
 * 一个待生成 NPC 的描述（纯数据，便于单测 roster）。
 * 位置是相对「锚点」（PlayerStart）的偏移，锚定地面 → 不依赖各关卡布局也能落在可达地面。
 */
struct FNpcSpawnSpec
{
	FName   NpcId;
	FText   SpeakerName;
	FString DialogueLine;     // 没对话树时的兜底气泡台词
	FVector OffsetFromAnchor; // 相对 PlayerStart 的偏移（厘米）
	float   YawDegrees = 0.f; // 朝向

	FNpcSpawnSpec() = default;
	FNpcSpawnSpec(FName InId, const FText& InSpeaker, const FString& InLine, const FVector& InOffset, float InYaw)
		: NpcId(InId), SpeakerName(InSpeaker), DialogueLine(InLine), OffsetFromAnchor(InOffset), YawDegrees(InYaw) {}
};

/**
 * 代码驱动的 NPC 填充。Plan 12。
 *
 * 关卡 BeginPlay 时按关卡名取 roster，把 NPC 生成在 PlayerStart 附近的地面上 ——
 * 让世界里有可对话的人，又不必手摆进二进制 .umap（内容可版本化、不开编辑器、即玩即见）。
 * 幂等：已存在同名 NpcId（如手摆的 Ah Hua）则跳过，与手摆 actor 共存。
 */
UCLASS()
class SGLIFESIM_API USGWorldPopulatorSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	/**
	 * 某关卡应当填充的 NPC 名单。纯函数、不碰世界 —— 供单测交叉校验
	 *（每个 NpcId 都得有注册的对话树）。LevelName 可含 PIE 前缀/路径，内部按子串匹配。
	 */
	static TArray<FNpcSpawnSpec> GetRosterForLevel(const FString& LevelName);

private:
	/** 仅在真正游戏/PIE 世界里填充。 */
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
};
