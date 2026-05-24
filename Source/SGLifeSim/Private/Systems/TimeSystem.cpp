#include "Systems/TimeSystem.h"

FTimeSystem::FTimeSystem()
	: TotalBlocksSinceStart(0)
{
}

void FTimeSystem::AdvanceBlock()
{
	++TotalBlocksSinceStart;
}

ETimeBlock FTimeSystem::GetCurrentBlock() const
{
	const int32 BlockIndex = TotalBlocksSinceStart % BlocksPerDay;
	return static_cast<ETimeBlock>(BlockIndex);
}

int32 FTimeSystem::GetDayNumber() const
{
	return 1 + (TotalBlocksSinceStart / BlocksPerDay);
}

EWeekday FTimeSystem::GetWeekday() const
{
	const int32 DayIndex = (GetDayNumber() - 1) % DaysPerWeek;
	return static_cast<EWeekday>(DayIndex);
}

int32 FTimeSystem::GetMonthNumber() const
{
	return 1 + ((GetDayNumber() - 1) / DaysPerMonth);
}

int32 FTimeSystem::GetDayOfMonth() const
{
	return 1 + ((GetDayNumber() - 1) % DaysPerMonth);
}
