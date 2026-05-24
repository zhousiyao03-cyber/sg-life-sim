#include "Systems/TimeSubsystem.h"

void UTimeSubsystem::AdvanceBlock()
{
	Time.AdvanceBlock();
	OnTimeAdvanced.Broadcast(Time.GetCurrentBlock(), Time.GetDayNumber());
}

ETimeBlock UTimeSubsystem::GetCurrentBlock() const
{
	return Time.GetCurrentBlock();
}

int32 UTimeSubsystem::GetDayNumber() const
{
	return Time.GetDayNumber();
}

EWeekday UTimeSubsystem::GetWeekday() const
{
	return Time.GetWeekday();
}

int32 UTimeSubsystem::GetMonthNumber() const
{
	return Time.GetMonthNumber();
}

int32 UTimeSubsystem::GetDayOfMonth() const
{
	return Time.GetDayOfMonth();
}
