#include "Systems/RelationshipSubsystem.h"

void URelationshipSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Relationship.OnRelationshipChanged.AddLambda([this](FName NpcId, int32 NewValue)
	{
		OnRelationshipChanged.Broadcast(NpcId, NewValue);
	});
}

void URelationshipSubsystem::AddAffinity(FName NpcId, int32 Delta)
{
	Relationship.AddAffinity(NpcId, Delta);
}

int32 URelationshipSubsystem::GetAffinity(FName NpcId) const
{
	return Relationship.GetAffinity(NpcId);
}

ERelationshipTier URelationshipSubsystem::GetTier(FName NpcId) const
{
	return Relationship.GetTier(NpcId);
}
