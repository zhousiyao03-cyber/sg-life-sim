#include "Systems/CareerSystem.h"

FCareerSystem::FCareerSystem()
{
	Level = ECareerLevel::Junior;
	GrossSalaryCents = BaseSalaryCents(ECareerLevel::Junior);
	MonthsInRole = 0;
}

int64 FCareerSystem::BaseSalaryCents(ECareerLevel InLevel)
{
	switch (InLevel)
	{
	case ECareerLevel::Junior:    return 500000;   // $5,000（spec 起点）
	case ECareerLevel::Mid:       return 750000;   // $7,500
	case ECareerLevel::Senior:    return 1100000;  // $11,000
	case ECareerLevel::Lead:      return 1600000;  // $16,000
	case ECareerLevel::Principal: return 2400000;  // $24,000
	default:                      return 0;        // 待业
	}
}

int32 FCareerSystem::PromotionProfessionalReq(ECareerLevel TargetLevel)
{
	switch (TargetLevel)
	{
	case ECareerLevel::Mid:       return 50;
	case ECareerLevel::Senior:    return 65;
	case ECareerLevel::Lead:      return 80;
	case ECareerLevel::Principal: return 92;
	default:                      return 0;  // Junior / Unemployed 无门槛
	}
}

ECareerLevel FCareerSystem::NextLevel() const
{
	switch (Level)
	{
	case ECareerLevel::Unemployed: return ECareerLevel::Junior;
	case ECareerLevel::Junior:     return ECareerLevel::Mid;
	case ECareerLevel::Mid:        return ECareerLevel::Senior;
	case ECareerLevel::Senior:     return ECareerLevel::Lead;
	case ECareerLevel::Lead:       return ECareerLevel::Principal;
	default:                       return ECareerLevel::Principal; // 已到顶
	}
}

bool FCareerSystem::CanPromote(int32 Professional) const
{
	if (Level == ECareerLevel::Principal)
	{
		return false; // 已到顶
	}
	const ECareerLevel Target = NextLevel();
	return Professional >= PromotionProfessionalReq(Target)
		&& MonthsInRole >= MinMonthsForPromotion;
}

bool FCareerSystem::Promote(int32 Professional)
{
	if (!CanPromote(Professional))
	{
		return false;
	}
	Level = NextLevel();
	// 升职不砍掉此前跳槽涨上去的薪资。
	GrossSalaryCents = FMath::Max(GrossSalaryCents, BaseSalaryCents(Level));
	MonthsInRole = 0;
	return true;
}

bool FCareerSystem::JobHop(int32 RaisePercent)
{
	if (!IsEmployed())
	{
		return false;
	}
	const int32 Pct = FMath::Max(0, RaisePercent);
	GrossSalaryCents += GrossSalaryCents * (int64)Pct / 100;
	MonthsInRole = 0;
	return true;
}

void FCareerSystem::RestoreState(ECareerLevel InLevel, int64 InGrossSalaryCents, int32 InMonthsInRole)
{
	Level = InLevel;
	GrossSalaryCents = FMath::Max((int64)0, InGrossSalaryCents);
	MonthsInRole = FMath::Max(0, InMonthsInRole);
}
