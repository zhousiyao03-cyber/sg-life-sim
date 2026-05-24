#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

#include "Systems/ActivitySubsystem.h"
#include "Systems/PlayerStateSubsystem.h"
#include "Systems/PlayerStatsTypes.h"
#include "Systems/EconomySubsystem.h"
#include "Systems/TimeSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace { constexpr int64 ACDollars(int64 D) { return D * 100; } }

/**
 * 活动循环端到端（Plan 10）：headless GameInstance 上验证活动改属性/现金、推进时间，
 * 能量是有效约束（耗尽被挡，睡觉恢复后又能做）。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FActivityLoopTest,
	"SGLifeSim.Integration.ActivityLoop",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FActivityLoopTest::RunTest(const FString& Parameters)
{
	UGameInstance* GI = NewObject<UGameInstance>(GEngine);
	if (!GI) { return false; }
	GI->InitializeStandalone();

	UActivitySubsystem*    Act  = GI->GetSubsystem<UActivitySubsystem>();
	UPlayerStateSubsystem* PS   = GI->GetSubsystem<UPlayerStateSubsystem>();
	UEconomySubsystem*     Eco  = GI->GetSubsystem<UEconomySubsystem>();
	UTimeSubsystem*        Time = GI->GetSubsystem<UTimeSubsystem>();
	if (!Act || !PS || !Eco || !Time) { GI->Shutdown(); return false; }

	const int32 StartProf = PS->GetAttribute(EPlayerAttribute::Professional);
	const int32 StartInsight = PS->GetAttribute(EPlayerAttribute::Insight);
	const int32 StartDay = Time->GetDayNumber();
	PS->SetAttribute(EPlayerAttribute::Energy, 100);

	// 学习：专业 +4、见识 +2、能量 -15（1 个时间块，仍在当天）。
	TestTrue(TEXT("study ok"), Act->PerformActivity(EActivityType::Study));
	TestEqual(TEXT("professional +4"), PS->GetAttribute(EPlayerAttribute::Professional), StartProf + 4);
	TestEqual(TEXT("insight +2"), PS->GetAttribute(EPlayerAttribute::Insight), StartInsight + 2);
	TestEqual(TEXT("energy 85"), PS->GetAttribute(EPlayerAttribute::Energy), 85);

	// 接私活：+$300，能量 -20 → 65。
	TestTrue(TEXT("freelance ok"), Act->PerformActivity(EActivityType::FreelanceCode));
	TestEqual(TEXT("cash $300"), Eco->GetBalance(ECurrencyAccount::Cash), ACDollars(300));
	TestEqual(TEXT("energy 65"), PS->GetAttribute(EPlayerAttribute::Energy), 65);

	// 能量门槛：直接把精力压到 10（模拟疲惫），接私活(需 20)被挡 —— 现金/精力不变、不推时间。
	PS->SetAttribute(EPlayerAttribute::Energy, 10);
	const int64 CashAt10 = Eco->GetBalance(ECurrencyAccount::Cash);
	TestFalse(TEXT("cannot code at energy 10"), Act->CanPerform(EActivityType::FreelanceCode));
	TestFalse(TEXT("perform blocked"), Act->PerformActivity(EActivityType::FreelanceCode));
	TestEqual(TEXT("cash unchanged when blocked"), Eco->GetBalance(ECurrencyAccount::Cash), CashAt10);
	TestEqual(TEXT("energy unchanged when blocked"), PS->GetAttribute(EPlayerAttribute::Energy), 10);

	// 精力够了又能接私活：+$300 → $600，能量 -20 → 80。
	PS->SetAttribute(EPlayerAttribute::Energy, 100);
	TestTrue(TEXT("code again ok"), Act->PerformActivity(EActivityType::FreelanceCode));
	TestEqual(TEXT("cash $600 total"), Eco->GetBalance(ECurrencyAccount::Cash), ACDollars(600));
	TestEqual(TEXT("energy 80"), PS->GetAttribute(EPlayerAttribute::Energy), 80);

	// 睡觉是恢复型活动，精力很低也能做。
	PS->SetAttribute(EPlayerAttribute::Energy, 5);
	TestTrue(TEXT("can always sleep"), Act->CanPerform(EActivityType::Sleep));
	TestTrue(TEXT("sleep ok"), Act->PerformActivity(EActivityType::Sleep));
	TestTrue(TEXT("energy recovered after sleep"), PS->GetAttribute(EPlayerAttribute::Energy) > 5);

	// 累计成功活动 = 3 块(study/code/code) + 2 块(sleep) = 5 块 → 跨一天。
	TestTrue(TEXT("time advanced at least a day"), Time->GetDayNumber() >= StartDay + 1);

	GI->Shutdown();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
