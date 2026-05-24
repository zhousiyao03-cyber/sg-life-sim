#include "Systems/RelationshipSystem.h"

void FRelationshipSystem::AddAffinity(FName NpcId, int32 Delta)
{
	if (NpcId.IsNone())
	{
		return;
	}

	int32& Value = Affinities.FindOrAdd(NpcId);
	Value = FMath::Clamp(Value + Delta, MinAffinity, MaxAffinity);
	OnRelationshipChanged.Broadcast(NpcId, Value);
}

int32 FRelationshipSystem::GetAffinity(FName NpcId) const
{
	const int32* Found = Affinities.Find(NpcId);
	return Found ? *Found : 0;
}

ERelationshipTier FRelationshipSystem::TierForAffinity(int32 Affinity)
{
	if (Affinity >= 90) return ERelationshipTier::Lover;
	if (Affinity >= 70) return ERelationshipTier::Confidant;
	if (Affinity >= 50) return ERelationshipTier::Friend;
	if (Affinity >= 30) return ERelationshipTier::Familiar;
	if (Affinity >= 10) return ERelationshipTier::Acquaintance;
	return ERelationshipTier::Stranger;
}

ERelationshipTier FRelationshipSystem::GetTier(FName NpcId) const
{
	return TierForAffinity(GetAffinity(NpcId));
}

void FRelationshipSystem::RestoreAffinities(const TMap<FName, int32>& InAffinities)
{
	Affinities = InAffinities;
}
