#pragma once

#include "CoreMinimal.h"
#include "Systems/RelationshipTypes.h"

/** 某 NPC 好感度变化时广播（原生多播，纯 C++ 可用）。 */
DECLARE_MULTICAST_DELEGATE_TwoParams(
	FOnRelationshipChanged, FName /*NpcId*/, int32 /*NewAffinity*/);

/**
 * 关系系统。spec §6.3。
 *
 * 纯 C++：用 TMap<FName, int32> 存每个 NPC 的好感度 0~100，零 UE 依赖、可单测。
 * 对话树留后续 plan，本系统只管好感数值 + 等级映射。
 */
class SGLIFESIM_API FRelationshipSystem
{
public:
	/** 好感度上下限。 */
	static constexpr int32 MinAffinity = 0;
	static constexpr int32 MaxAffinity = 100;

	/** 增减好感（可负），结果 clamp 到 [0,100]。首次接触会创建记录。 */
	void AddAffinity(FName NpcId, int32 Delta);

	/** 查询好感度。未接触过的 NPC 返回 0（陌生）。 */
	int32 GetAffinity(FName NpcId) const;

	/** 好感度对应的关系等级。 */
	ERelationshipTier GetTier(FName NpcId) const;

	/** 由一个好感数值算等级（静态，便于测试 / 复用）。 */
	static ERelationshipTier TierForAffinity(int32 Affinity);

	/** 全部好感记录（存档 / UI 用）。 */
	const TMap<FName, int32>& GetAllAffinities() const { return Affinities; }

	/** 从存档恢复。 */
	void RestoreAffinities(const TMap<FName, int32>& InAffinities);

	/** 好感变化时触发。 */
	FOnRelationshipChanged OnRelationshipChanged;

private:
	TMap<FName, int32> Affinities;
};
