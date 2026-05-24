#include "Systems/PlayerStats.h"

FPlayerStats::FPlayerStats()
{
	// 初始属性：刚落地新加坡的程序员（健康好、能量满、专业过硬、社交/见识偏低）。
	Values[(int32)EPlayerAttribute::Health]       = 80;
	Values[(int32)EPlayerAttribute::Mood]         = 60;
	Values[(int32)EPlayerAttribute::Energy]       = 100;
	Values[(int32)EPlayerAttribute::Professional] = 60;
	Values[(int32)EPlayerAttribute::Social]       = 40;
	Values[(int32)EPlayerAttribute::Insight]      = 30;
}

int32 FPlayerStats::Get(EPlayerAttribute Attr) const
{
	const int32 Index = (int32)Attr;
	if (Index < 0 || Index >= (int32)EPlayerAttribute::Count)
	{
		return 0;
	}
	return Values[Index];
}

void FPlayerStats::Set(EPlayerAttribute Attr, int32 Value)
{
	const int32 Index = (int32)Attr;
	if (Index < 0 || Index >= (int32)EPlayerAttribute::Count)
	{
		return;
	}
	Values[Index] = FMath::Clamp(Value, MinValue, MaxValue);
}

void FPlayerStats::Modify(EPlayerAttribute Attr, int32 Delta)
{
	Set(Attr, Get(Attr) + Delta);
}

void FPlayerStats::RestoreEnergyDaily()
{
	Set(EPlayerAttribute::Energy, MaxValue);
}

TArray<int32> FPlayerStats::GetSnapshot() const
{
	TArray<int32> Out;
	Out.Reserve((int32)EPlayerAttribute::Count);
	for (int32 i = 0; i < (int32)EPlayerAttribute::Count; ++i)
	{
		Out.Add(Values[i]);
	}
	return Out;
}

void FPlayerStats::RestoreSnapshot(const TArray<int32>& Snapshot)
{
	if (Snapshot.Num() != (int32)EPlayerAttribute::Count)
	{
		return;  // 长度不符，忽略（防脏存档）
	}
	for (int32 i = 0; i < (int32)EPlayerAttribute::Count; ++i)
	{
		Values[i] = FMath::Clamp(Snapshot[i], MinValue, MaxValue);
	}
}
