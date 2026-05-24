#include "Systems/NightCommuteSubsystem.h"
#include "Systems/NightCommuteSystem.h"
#include "Systems/HorrorEventSubsystem.h"
#include "Systems/TimeSubsystem.h"
#include "Systems/TimeBlock.h"
#include "Systems/SanitySubsystem.h"
#include "Systems/PlayerStateSubsystem.h"
#include "Systems/PlayerStatsTypes.h"

void UNightCommuteSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency(UHorrorEventSubsystem::StaticClass());
	Collection.InitializeDependency(USanitySubsystem::StaticClass());

	// 默认随时间变化的种子；测试可 SetSeed 覆盖。
	Stream.Initialize((int32)(FDateTime::Now().GetTicks() & 0x7fffffff));
}

bool UNightCommuteSubsystem::IsAvailable() const
{
	const UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return false;
	}

	// 仅农历七月深夜可触发（电梯停在 13 楼那回事）。
	const UHorrorEventSubsystem* Horror = GI->GetSubsystem<UHorrorEventSubsystem>();
	const UTimeSubsystem* Time = GI->GetSubsystem<UTimeSubsystem>();
	if (!Horror || !Time)
	{
		return false;
	}
	return Horror->IsGhostMonth() && Time->GetCurrentBlock() == ETimeBlock::LateNight;
}

FNightCommuteOutcome UNightCommuteSubsystem::MakeChoice(ENightCommuteChoice Choice)
{
	const FNightCommuteOutcome Out = FNightCommuteSystem::Resolve(Choice, Stream);

	if (UGameInstance* GI = GetGameInstance())
	{
		if (Out.SanityDelta != 0)
		{
			if (USanitySubsystem* San = GI->GetSubsystem<USanitySubsystem>())
			{
				if (Out.SanityDelta > 0) { San->Restore(Out.SanityDelta); }
				else                     { San->Drain(-Out.SanityDelta); }
			}
		}
		if (Out.EnergyDelta != 0)
		{
			if (UPlayerStateSubsystem* PS = GI->GetSubsystem<UPlayerStateSubsystem>())
			{
				PS->ModifyAttribute(EPlayerAttribute::Energy, Out.EnergyDelta);
			}
		}
	}

	OnResolved.Broadcast(Out.Message);
	return Out;
}
