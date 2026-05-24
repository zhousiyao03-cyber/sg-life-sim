#include "Systems/RoadsideOfferingSubsystem.h"
#include "Systems/RoadsideOfferingSystem.h"
#include "Systems/HorrorEventSubsystem.h"
#include "Systems/HorrorEventTypes.h"
#include "Systems/HorrorCodexSubsystem.h"
#include "Systems/TimeSubsystem.h"
#include "Systems/TimeBlock.h"
#include "Systems/SanitySubsystem.h"
#include "Systems/PlayerStateSubsystem.h"
#include "Systems/PlayerStatsTypes.h"
#include "Systems/ProgressSubsystem.h"
#include "Systems/SGAchievementIds.h"

void URoadsideOfferingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency(UHorrorEventSubsystem::StaticClass());
	Collection.InitializeDependency(USanitySubsystem::StaticClass());

	// 默认随时间变化的种子；测试可 SetSeed 覆盖。
	Stream.Initialize((int32)(FDateTime::Now().GetTicks() & 0x7fffffff));
}

bool URoadsideOfferingSubsystem::IsAvailable() const
{
	const UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return false;
	}

	// 仅农历七月深夜可触发（路中间那堆金纸）。复用 HorrorEvent 的鬼月判定。
	const UHorrorEventSubsystem* Horror = GI->GetSubsystem<UHorrorEventSubsystem>();
	const UTimeSubsystem* Time = GI->GetSubsystem<UTimeSubsystem>();
	if (!Horror || !Time)
	{
		return false;
	}
	return Horror->IsGhostMonth() && Time->GetCurrentBlock() == ETimeBlock::LateNight;
}

FRoadsideOfferingOutcome URoadsideOfferingSubsystem::MakeChoice(ERoadsideOfferingChoice Choice)
{
	const FRoadsideOfferingOutcome Out = FRoadsideOfferingSystem::Resolve(Choice, Stream);

	UGameInstance* GI = GetGameInstance();

	// 精力代价无论哪条路都扣（绕路 / 跨过 / 拜拜都耗精力）。
	if (GI && Out.EnergyDelta != 0)
	{
		if (UPlayerStateSubsystem* PS = GI->GetSubsystem<UPlayerStateSubsystem>())
		{
			PS->ModifyAttribute(EPlayerAttribute::Energy, Out.EnergyDelta);
		}
	}

	// 理智结算（博弈自己的惩罚 / 奖励，单扣）。
	if (GI && Out.SanityDelta != 0)
	{
		if (USanitySubsystem* San = GI->GetSubsystem<USanitySubsystem>())
		{
			if (Out.SanityDelta > 0) { San->Restore(Out.SanityDelta); }
			else                     { San->Drain(-Out.SanityDelta); }
		}
	}

	// 赌输（跨过去犯了冥纸禁忌、招了事）→ 记进恐怖图鉴（ZhiQianTaboo），
	// 与 AhMei 的「冥纸共鸣」对话闭环（赌输 → 图鉴有冥纸 → 可向阿姨坦白被安抚）。
	// 注意：理智已由上面的博弈惩罚扣过，这里只记图鉴，不再走 ApplyEvent 二次扣理智。
	if (Out.bSomethingHappened && GI)
	{
		if (UHorrorCodexSubsystem* Codex = GI->GetSubsystem<UHorrorCodexSubsystem>())
		{
			Codex->RecordEncounter(EHorrorEvent::ZhiQianTaboo);
		}
	}

	// 懂得敬畏：守规矩（不去赌的安全选项：绕开 / 拜一拜）首次达成解锁成就。
	if (GI && (Choice == ERoadsideOfferingChoice::DetourAround || Choice == ERoadsideOfferingChoice::PayRespects))
	{
		if (UProgressSubsystem* Prog = GI->GetSubsystem<UProgressSubsystem>())
		{
			Prog->MarkAchieved(SGAchievementIds::RespectTheUnseen());
		}
	}

	OnResolved.Broadcast(Out.Message);
	return Out;
}
