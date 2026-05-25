#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Systems/WantedSubsystem.h"
#include "Systems/EconomySubsystem.h"
#include "Systems/EconomyTypes.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"

#if WITH_DEV_AUTOMATION_TESTS

// 通缉系统的回归测试（覆盖此前"打一拳就满星"和"警察免费销案"两个 bug 的修复）。
// UWantedSubsystem 是 GameInstanceSubsystem，ClassWithin=GameInstance，不能裸 NewObject——
// 必须经真 GameInstance + InitializeStandalone 拿子系统（沿用本项目集成测试惯例）。

namespace
{
	// 起一个独立 GameInstance 并取出通缉/经济子系统。沿用 CoreSystemsIntegrationTest 的写法。
	UGameInstance* MakeGI(UWantedSubsystem*& OutWanted, UEconomySubsystem*& OutEcon)
	{
		UGameInstance* GI = NewObject<UGameInstance>(GEngine);
		GI->AddToRoot();
		GI->InitializeStandalone();
		OutWanted = GI->GetSubsystem<UWantedSubsystem>();
		OutEcon = GI->GetSubsystem<UEconomySubsystem>();
		return GI;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWantedHeatToStarsTest,
	"SGLifeSim.Wanted.HeatToStars",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWantedHeatToStarsTest::RunTest(const FString& Parameters)
{
	UWantedSubsystem* W = nullptr; UEconomySubsystem* E = nullptr;
	UGameInstance* GI = MakeGI(W, E);
	TestNotNull(TEXT("wanted subsystem"), W);
	if (!W) { GI->RemoveFromRoot(); return false; }

	TestEqual(TEXT("起步 0 星"), W->GetStars(), 0);

	W->AddHeat(99);
	TestEqual(TEXT("99 heat 仍 0 星"), W->GetStars(), 0);
	W->AddHeat(1);
	TestEqual(TEXT("100 heat = 1 星"), W->GetStars(), 1);

	W->AddHeat(99999);
	TestEqual(TEXT("封顶 5 星"), W->GetStars(), 5);
	TestEqual(TEXT("heat 封到 500"), W->GetHeat(), 500);

	GI->RemoveFromRoot();
	return true;
}

// 核心回归：一拳打路人不该瞬间满星（动手 8 + 一次目击举报 15 = 23，远不到 1 星）。
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWantedPunchDoesNotMaxTest,
	"SGLifeSim.Wanted.PunchDoesNotMax",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWantedPunchDoesNotMaxTest::RunTest(const FString& Parameters)
{
	UWantedSubsystem* W = nullptr; UEconomySubsystem* E = nullptr;
	UGameInstance* GI = MakeGI(W, E);
	if (!W) { GI->RemoveFromRoot(); return false; }

	W->AddHeat(8);                            // TakeMeleeHit 对路人
	const bool bReported = W->ReportCrime(15, /*Now=*/1.0f);
	TestTrue(TEXT("首次目击成功举报"), bReported);
	TestEqual(TEXT("打一个路人累计 23 heat"), W->GetHeat(), 23);
	TestEqual(TEXT("打一个路人 0 星（远不该满）"), W->GetStars(), 0);

	GI->RemoveFromRoot();
	return true;
}

// 目击举报全局限频：一群路人同时看到同一桩事，只该记一笔。
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWantedReportThrottleTest,
	"SGLifeSim.Wanted.ReportThrottle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWantedReportThrottleTest::RunTest(const FString& Parameters)
{
	UWantedSubsystem* W = nullptr; UEconomySubsystem* E = nullptr;
	UGameInstance* GI = MakeGI(W, E);
	if (!W) { GI->RemoveFromRoot(); return false; }

	TestTrue(TEXT("路人A举报生效"), W->ReportCrime(15, 5.0f));
	TestFalse(TEXT("路人B同刻举报被吞"), W->ReportCrime(15, 5.0f));
	TestFalse(TEXT("路人C冷却窗口内举报被吞"), W->ReportCrime(15, 7.0f));
	TestEqual(TEXT("一桩事只记一笔 15"), W->GetHeat(), 15);

	TestTrue(TEXT("冷却后（>3s）新举报生效"), W->ReportCrime(15, 9.0f));
	TestEqual(TEXT("两桩事共 30"), W->GetHeat(), 30);

	GI->RemoveFromRoot();
	return true;
}

// 核心回归：被捕要交保释金（扣钱）后才清通缉，不再免费销案。
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWantedArrestChargesBailTest,
	"SGLifeSim.Wanted.ArrestChargesBail",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWantedArrestChargesBailTest::RunTest(const FString& Parameters)
{
	UWantedSubsystem* W = nullptr; UEconomySubsystem* E = nullptr;
	UGameInstance* GI = MakeGI(W, E);
	TestNotNull(TEXT("economy subsystem"), E);
	if (!W || !E) { GI->RemoveFromRoot(); return false; }

	// 给足现金，制造 2 星通缉。
	E->Deposit(ECurrencyAccount::Cash, 100000, TEXT("TestSeed")); // S$1000
	const int64 CashBefore = E->GetBalance(ECurrencyAccount::Cash);
	W->AddHeat(250);
	TestEqual(TEXT("逮捕前 2 星"), W->GetStars(), 2);

	W->Arrest();
	TestEqual(TEXT("逮捕后清零"), W->GetHeat(), 0);
	TestEqual(TEXT("逮捕扣了保释金"),
		E->GetBalance(ECurrencyAccount::Cash), CashBefore - UWantedSubsystem::ArrestBailCents);

	// 无通缉时逮捕是空操作（不扣钱、不改状态）。
	const int64 CashAfter = E->GetBalance(ECurrencyAccount::Cash);
	W->Arrest();
	TestEqual(TEXT("无通缉逮捕不扣钱"), E->GetBalance(ECurrencyAccount::Cash), CashAfter);

	GI->RemoveFromRoot();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
