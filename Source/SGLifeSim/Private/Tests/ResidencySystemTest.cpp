#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Systems/ResidencySystem.h"
#include "Systems/ResidencyTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FResidencyStartsOnEPTest,
	"SGLifeSim.Residency.StartsOnEP",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FResidencyStartsOnEPTest::RunTest(const FString& Parameters)
{
	FResidencySystem Sys;
	TestEqual(TEXT("starts on EP"), Sys.GetStatus(), EResidencyStatus::WorkPermit_EP);
	TestTrue(TEXT("is on work permit"), Sys.IsOnWorkPermit());
	TestEqual(TEXT("no PR rejections"), Sys.GetPRRejectionCount(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FResidencyApprovalPathTest,
	"SGLifeSim.Residency.ApprovalToCitizen",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FResidencyApprovalPathTest::RunTest(const FString& Parameters)
{
	FResidencySystem Sys;
	TestTrue(TEXT("can apply from EP"), Sys.ApplyForPR());
	TestEqual(TEXT("now applying"), Sys.GetStatus(), EResidencyStatus::PR_Applying);

	TestTrue(TEXT("resolve handled"), Sys.ResolvePRApplication(true));
	TestEqual(TEXT("approved -> PR"), Sys.GetStatus(), EResidencyStatus::PR);

	TestTrue(TEXT("PR can naturalize"), Sys.Naturalize());
	TestEqual(TEXT("now citizen"), Sys.GetStatus(), EResidencyStatus::Citizen);
	TestEqual(TEXT("no rejections on clean path"), Sys.GetPRRejectionCount(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FResidencyRejectionPathTest,
	"SGLifeSim.Residency.RejectionReturnsToPermit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FResidencyRejectionPathTest::RunTest(const FString& Parameters)
{
	FResidencySystem Sys;
	Sys.RestoreState(EResidencyStatus::WorkPermit_SP, 0);  // 从 SP 起
	TestTrue(TEXT("apply from SP"), Sys.ApplyForPR());

	TestTrue(TEXT("resolve handled"), Sys.ResolvePRApplication(false));
	TestEqual(TEXT("rejected -> back to SP"), Sys.GetStatus(), EResidencyStatus::WorkPermit_SP);
	TestEqual(TEXT("rejection counted"), Sys.GetPRRejectionCount(), 1);

	// 可再次申请；这次通过
	TestTrue(TEXT("reapply"), Sys.ApplyForPR());
	TestTrue(TEXT("resolve"), Sys.ResolvePRApplication(true));
	TestEqual(TEXT("now PR"), Sys.GetStatus(), EResidencyStatus::PR);
	TestEqual(TEXT("rejection count persists"), Sys.GetPRRejectionCount(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FResidencyIllegalTransitionsTest,
	"SGLifeSim.Residency.IllegalTransitionsRejected",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FResidencyIllegalTransitionsTest::RunTest(const FString& Parameters)
{
	FResidencySystem Sys;
	// EP 不能直接入籍
	TestFalse(TEXT("EP cannot naturalize"), Sys.Naturalize());
	// 没在申请中时裁决无效
	TestFalse(TEXT("resolve without applying fails"), Sys.ResolvePRApplication(true));
	// 申请中不能再次申请
	Sys.ApplyForPR();
	TestFalse(TEXT("cannot re-apply while applying"), Sys.ApplyForPR());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
