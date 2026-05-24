#include "Systems/NightCommuteSubsystem.h"
#include "Systems/NightCommuteSystem.h"
#include "Systems/HorrorEventSubsystem.h"
#include "Systems/TimeSubsystem.h"
#include "Systems/TimeBlock.h"
#include "Systems/SanitySubsystem.h"
#include "Systems/PlayerStateSubsystem.h"
#include "Systems/PlayerStatsTypes.h"
#include "Systems/HorrorSequenceSubsystem.h"
#include "Systems/HorrorSceneTypes.h"

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

	UGameInstance* GI = GetGameInstance();

	// 精力代价无论哪条路都扣（进去这一趟 / 等 / 爬楼梯都耗精力）。
	if (GI && Out.EnergyDelta != 0)
	{
		if (UPlayerStateSubsystem* PS = GI->GetSubsystem<UPlayerStateSubsystem>())
		{
			PS->ModifyAttribute(EPlayerAttribute::Energy, Out.EnergyDelta);
		}
	}

	// 赌输（犯禁忌进了电梯、招了事）→ 升级为真场景演出（Plan 24）：
	// 把你拖进电梯恐怖场景，理智 / 图鉴交由场景结算，这里不再扣理智、也不弹
	// 「身后有人」文案（改由场景演出 + 事后文案呈现）。
	if (Out.bSomethingHappened && GI)
	{
		if (UHorrorSequenceSubsystem* Seq = GI->GetSubsystem<UHorrorSequenceSubsystem>())
		{
			if (Seq->EnterScene(EHorrorScene::Elevator))
			{
				return Out; // 已进场景：不走常规理智结算 / 文案广播。
			}
		}
		// EnterScene 失败（无 World / 已在场景中）→ 退回常规理智结算，保证不漏处理。
	}

	// 常规理智结算（等下一趟 / 走楼梯 / 赌赢 / 赌输但没进成场景）。
	if (GI && Out.SanityDelta != 0)
	{
		if (USanitySubsystem* San = GI->GetSubsystem<USanitySubsystem>())
		{
			if (Out.SanityDelta > 0) { San->Restore(Out.SanityDelta); }
			else                     { San->Drain(-Out.SanityDelta); }
		}
	}

	OnResolved.Broadcast(Out.Message);
	return Out;
}
