#include "Systems/HorrorEventSubsystem.h"
#include "Systems/HorrorEventSystem.h"

#include "Systems/TimeSubsystem.h"
#include "Systems/PlayerStateSubsystem.h"
#include "Systems/PlayerStatsTypes.h"
#include "Systems/SanitySubsystem.h"
#include "Systems/SanityTypes.h"
#include "Systems/HorrorSequenceSubsystem.h"
#include "Systems/HorrorSceneTypes.h"

void UHorrorEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency(UTimeSubsystem::StaticClass());
	Collection.InitializeDependency(UPlayerStateSubsystem::StaticClass());
	Collection.InitializeDependency(USanitySubsystem::StaticClass());

	// 默认随时间变化的种子，让每局不同；测试可 SetSeed 覆盖。
	Stream.Initialize((int32)(FDateTime::Now().GetTicks() & 0x7fffffff));

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UTimeSubsystem* TimeSys = GI->GetSubsystem<UTimeSubsystem>())
		{
			TimeSys->OnTimeAdvanced.AddDynamic(this, &UHorrorEventSubsystem::HandleTimeAdvanced);
			LastRolledDay = TimeSys->GetDayNumber();
		}
	}
}

void UHorrorEventSubsystem::Deinitialize()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UTimeSubsystem* TimeSys = GI->GetSubsystem<UTimeSubsystem>())
		{
			TimeSys->OnTimeAdvanced.RemoveDynamic(this, &UHorrorEventSubsystem::HandleTimeAdvanced);
		}
	}
	Super::Deinitialize();
}

bool UHorrorEventSubsystem::IsGhostMonth() const
{
	UGameInstance* GI = GetGameInstance();
	UTimeSubsystem* TimeSys = GI ? GI->GetSubsystem<UTimeSubsystem>() : nullptr;
	return TimeSys ? FHorrorEventSystem::IsGhostMonth(TimeSys->GetMonthNumber()) : false;
}

void UHorrorEventSubsystem::HandleTimeAdvanced(ETimeBlock NewBlock, int32 DayNumber)
{
	if (!bEnabled || NewBlock != ETimeBlock::LateNight)
	{
		return;
	}
	// 每天入夜只掷一次。
	if (DayNumber == LastRolledDay)
	{
		return;
	}
	LastRolledDay = DayNumber;

	const bool bGhost = IsGhostMonth();
	int32 DreadBonus = 0;
	bool bLowSanity = false;
	if (USanitySubsystem* Sanity = GetGameInstance()->GetSubsystem<USanitySubsystem>())
	{
		DreadBonus = Sanity->GetExtraDreadWeight();
		// 理智「失常」档及以下才会「看见」幻觉。
		bLowSanity = (uint8)Sanity->GetState() >= (uint8)ESanityState::Disturbed;
	}
	// 每天深夜必出一条恐怖事件（「无事」不入池）；抽哪条仍按权重随机，鬼月/幻觉门控照旧。
	const EHorrorEvent Picked = FHorrorEventSystem::PickEvent(Stream, bGhost, DreadBonus, bLowSanity, /*bGuaranteeEvent=*/true);

	// 电梯空楼层升级为真场景演出（Plan 24）：进电梯关卡，理智 / 图鉴由场景结算。
	// EnterScene 失败（无 World / 已在场景中）则退回常规 ApplyEvent，保证不漏处理。
	if (Picked == EHorrorEvent::ElevatorGhostFloor)
	{
		if (UHorrorSequenceSubsystem* Seq = GetGameInstance()->GetSubsystem<UHorrorSequenceSubsystem>())
		{
			LastEvent = Picked;
			if (Seq->EnterScene(EHorrorScene::Elevator))
			{
				return;
			}
		}
	}

	ApplyEvent(Picked);
}

bool UHorrorEventSubsystem::ApplyEvent(EHorrorEvent Event)
{
	LastEvent = Event;
	if (Event == EHorrorEvent::None)
	{
		return false;
	}

	const FHorrorEventDef Def = FHorrorEventSystem::GetEventDef(Event);

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UPlayerStateSubsystem* PS = GI->GetSubsystem<UPlayerStateSubsystem>())
		{
			if (Def.MoodDelta != 0)
			{
				PS->ModifyAttribute(EPlayerAttribute::Mood, Def.MoodDelta);
			}
			if (Def.HealthDelta != 0)
			{
				PS->ModifyAttribute(EPlayerAttribute::Health, Def.HealthDelta);
			}
		}
		if (USanitySubsystem* Sanity = GI->GetSubsystem<USanitySubsystem>())
		{
			if (Def.SanityCost != 0)
			{
				Sanity->Drain(Def.SanityCost);
			}
		}
	}

	OnHorrorEvent.Broadcast(Def.Title);
	OnHorrorEventTyped.Broadcast(Event);
	return true;
}
