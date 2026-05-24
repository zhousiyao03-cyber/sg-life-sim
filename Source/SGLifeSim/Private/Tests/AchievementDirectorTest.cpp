#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

#include "Systems/EconomySubsystem.h"
#include "Systems/RelationshipSubsystem.h"
#include "Systems/ProgressSubsystem.h"
#include "Systems/AchievementDirector.h"
#include "Systems/SGAchievementIds.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * 成就导演端到端：headless GameInstance 上 Director 订阅经济/关系委托，
 * 验证发薪→FirstSalary、净资产过 $10k→NetWorth10k、好感达朋友→FirstFriend 自动解锁。
 * Plan 3 Task 3。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAchievementDirectorTest,
	"SGLifeSim.Integration.AchievementUnlocks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAchievementDirectorTest::RunTest(const FString& Parameters)
{
	UGameInstance* GI = NewObject<UGameInstance>(GEngine);
	if (!GI) { return false; }
	GI->InitializeStandalone();  // Director 在此自动 Initialize + 订阅

	UEconomySubsystem*      Eco  = GI->GetSubsystem<UEconomySubsystem>();
	URelationshipSubsystem* Rel  = GI->GetSubsystem<URelationshipSubsystem>();
	UProgressSubsystem*     Prog = GI->GetSubsystem<UProgressSubsystem>();
	UAchievementDirector*   Dir  = GI->GetSubsystem<UAchievementDirector>();

	TestNotNull(TEXT("EconomySubsystem"), Eco);
	TestNotNull(TEXT("RelationshipSubsystem"), Rel);
	TestNotNull(TEXT("ProgressSubsystem"), Prog);
	TestNotNull(TEXT("AchievementDirector"), Dir);
	if (!Eco || !Rel || !Prog || !Dir) { GI->Shutdown(); return false; }

	// 起点：什么都没解锁。
	TestEqual(TEXT("no achievements at start"), Prog->GetAchievedCount(), 0);

	// 发薪 → FirstSalary；此时净资产 $5850 < $10k，NetWorth10k 还不解锁。
	Eco->ApplyMonthlySalary((int64)500000);
	TestTrue(TEXT("FirstSalary unlocked after salary"),
		Prog->HasAchieved(SGAchievementIds::FirstSalary()));
	TestFalse(TEXT("NetWorth10k not yet"),
		Prog->HasAchieved(SGAchievementIds::NetWorth10k()));

	// 存入银行把净资产推过 $10k → NetWorth10k。
	Eco->Deposit(ECurrencyAccount::Bank, (int64)500000, TEXT("Bonus"));
	TestTrue(TEXT("NetWorth10k unlocked past $10k"),
		Prog->HasAchieved(SGAchievementIds::NetWorth10k()));

	// 好感拉到「朋友」档（50） → FirstFriend。
	Rel->AddAffinity(TEXT("Auntie"), 55);
	TestTrue(TEXT("FirstFriend unlocked at Friend tier"),
		Prog->HasAchieved(SGAchievementIds::FirstFriend()));

	TestEqual(TEXT("exactly 3 achievements"), Prog->GetAchievedCount(), 3);

	GI->Shutdown();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
