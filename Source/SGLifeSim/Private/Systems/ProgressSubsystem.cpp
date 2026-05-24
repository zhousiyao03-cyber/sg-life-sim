#include "Systems/ProgressSubsystem.h"

void UProgressSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 把纯 C++ 核心的原生委托桥接到 BP 动态委托。
	Progress.OnAchievementUnlocked.AddLambda([this](FName Id)
	{
		OnAchievementUnlocked.Broadcast(Id);
	});
}

bool UProgressSubsystem::MarkAchieved(FName AchievementId)
{
	return Progress.MarkAchieved(AchievementId);
}

bool UProgressSubsystem::HasAchieved(FName AchievementId) const
{
	return Progress.HasAchieved(AchievementId);
}

int32 UProgressSubsystem::GetAchievedCount() const
{
	return Progress.GetAchievedCount();
}
