#include "Systems/AchievementDirector.h"
#include "Systems/EconomySubsystem.h"
#include "Systems/RelationshipSubsystem.h"
#include "Systems/ProgressSubsystem.h"
#include "Systems/RelationshipSystem.h"
#include "Systems/RelationshipTypes.h"
#include "Systems/SGAchievementIds.h"

void UAchievementDirector::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 先确保依赖的子系统已初始化，再订阅它们的委托。
	Collection.InitializeDependency(UEconomySubsystem::StaticClass());
	Collection.InitializeDependency(URelationshipSubsystem::StaticClass());
	Collection.InitializeDependency(UProgressSubsystem::StaticClass());

	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}
	if (UEconomySubsystem* Eco = GI->GetSubsystem<UEconomySubsystem>())
	{
		Eco->OnBalanceChanged.AddDynamic(this, &UAchievementDirector::HandleBalanceChanged);
	}
	if (URelationshipSubsystem* Rel = GI->GetSubsystem<URelationshipSubsystem>())
	{
		Rel->OnRelationshipChanged.AddDynamic(this, &UAchievementDirector::HandleRelationshipChanged);
	}
}

void UAchievementDirector::Deinitialize()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UEconomySubsystem* Eco = GI->GetSubsystem<UEconomySubsystem>())
		{
			Eco->OnBalanceChanged.RemoveDynamic(this, &UAchievementDirector::HandleBalanceChanged);
		}
		if (URelationshipSubsystem* Rel = GI->GetSubsystem<URelationshipSubsystem>())
		{
			Rel->OnRelationshipChanged.RemoveDynamic(this, &UAchievementDirector::HandleRelationshipChanged);
		}
	}
	Super::Deinitialize();
}

void UAchievementDirector::HandleBalanceChanged(ECurrencyAccount Account, int64 NewBalanceCents)
{
	EvaluateEconomyAchievements();
}

void UAchievementDirector::EvaluateEconomyAchievements()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}
	UEconomySubsystem* Eco = GI->GetSubsystem<UEconomySubsystem>();
	UProgressSubsystem* Prog = GI->GetSubsystem<UProgressSubsystem>();
	if (!Eco || !Prog)
	{
		return;
	}

	// 首次发薪：流水里出现过 "Salary"。
	static const FName SalaryReason(TEXT("Salary"));
	if (!Prog->HasAchieved(SGAchievementIds::FirstSalary()))
	{
		for (const FMoneyTransaction& Tx : Eco->GetEconomy().GetTransactions())
		{
			if (Tx.Reason == SalaryReason)
			{
				Prog->MarkAchieved(SGAchievementIds::FirstSalary());
				break;
			}
		}
	}

	// 净资产首次达 $10k。
	if (Eco->GetNetWorth() >= SGAchievementIds::NetWorth10kThresholdCents)
	{
		Prog->MarkAchieved(SGAchievementIds::NetWorth10k());
	}
}

void UAchievementDirector::HandleRelationshipChanged(FName NpcId, int32 NewAffinity)
{
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}
	UProgressSubsystem* Prog = GI->GetSubsystem<UProgressSubsystem>();
	if (!Prog)
	{
		return;
	}

	// 达到「朋友」档（含以上）→ 第一个朋友。
	if (FRelationshipSystem::TierForAffinity(NewAffinity) >= ERelationshipTier::Friend)
	{
		Prog->MarkAchieved(SGAchievementIds::FirstFriend());
	}
}
