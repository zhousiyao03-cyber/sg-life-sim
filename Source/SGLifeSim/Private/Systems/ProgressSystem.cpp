#include "Systems/ProgressSystem.h"

bool FProgressSystem::MarkAchieved(FName AchievementId)
{
	if (AchievementId.IsNone())
	{
		return false;
	}

	bool bAlready = false;
	Achieved.Add(AchievementId, &bAlready);
	if (!bAlready)
	{
		OnAchievementUnlocked.Broadcast(AchievementId);
		return true;  // 首次达成
	}
	return false;
}

bool FProgressSystem::HasAchieved(FName AchievementId) const
{
	return Achieved.Contains(AchievementId);
}

void FProgressSystem::RestoreAchieved(const TArray<FName>& Ids)
{
	Achieved.Reset();
	Achieved.Append(Ids);
}
