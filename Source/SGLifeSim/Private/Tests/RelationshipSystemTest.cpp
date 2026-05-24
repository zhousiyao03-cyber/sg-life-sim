#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Systems/RelationshipSystem.h"
#include "Systems/RelationshipTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRelationshipDefaultsStrangerTest,
	"SGLifeSim.Relationship.UnknownIsStranger",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRelationshipDefaultsStrangerTest::RunTest(const FString& Parameters)
{
	FRelationshipSystem Sys;
	TestEqual(TEXT("unknown NPC affinity 0"), Sys.GetAffinity(TEXT("Auntie")), 0);
	TestEqual(TEXT("unknown NPC tier Stranger"),
		Sys.GetTier(TEXT("Auntie")), ERelationshipTier::Stranger);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRelationshipAddAndClampTest,
	"SGLifeSim.Relationship.AddAndClamp",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRelationshipAddAndClampTest::RunTest(const FString& Parameters)
{
	FRelationshipSystem Sys;
	Sys.AddAffinity(TEXT("AhHua"), 25);
	TestEqual(TEXT("affinity now 25"), Sys.GetAffinity(TEXT("AhHua")), 25);

	Sys.AddAffinity(TEXT("AhHua"), 1000);  // 超上限
	TestEqual(TEXT("clamped to 100"), Sys.GetAffinity(TEXT("AhHua")), 100);

	Sys.AddAffinity(TEXT("AhHua"), -1000); // 超下限
	TestEqual(TEXT("clamped to 0"), Sys.GetAffinity(TEXT("AhHua")), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRelationshipTierThresholdsTest,
	"SGLifeSim.Relationship.TierThresholds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRelationshipTierThresholdsTest::RunTest(const FString& Parameters)
{
	using R = FRelationshipSystem;
	TestEqual(TEXT("0 -> Stranger"),     R::TierForAffinity(0),   ERelationshipTier::Stranger);
	TestEqual(TEXT("9 -> Stranger"),     R::TierForAffinity(9),   ERelationshipTier::Stranger);
	TestEqual(TEXT("10 -> Acquaintance"),R::TierForAffinity(10),  ERelationshipTier::Acquaintance);
	TestEqual(TEXT("30 -> Familiar"),    R::TierForAffinity(30),  ERelationshipTier::Familiar);
	TestEqual(TEXT("50 -> Friend"),      R::TierForAffinity(50),  ERelationshipTier::Friend);
	TestEqual(TEXT("70 -> Confidant"),   R::TierForAffinity(70),  ERelationshipTier::Confidant);
	TestEqual(TEXT("90 -> Lover"),       R::TierForAffinity(90),  ERelationshipTier::Lover);
	TestEqual(TEXT("100 -> Lover"),      R::TierForAffinity(100), ERelationshipTier::Lover);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRelationshipRestoreTest,
	"SGLifeSim.Relationship.RestoreFromSave",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRelationshipRestoreTest::RunTest(const FString& Parameters)
{
	FRelationshipSystem Sys;
	TMap<FName, int32> Saved;
	Saved.Add(TEXT("Raj"), 65);
	Saved.Add(TEXT("ZhangSir"), 40);
	Sys.RestoreAffinities(Saved);

	TestEqual(TEXT("restored Raj 65 -> Friend"), Sys.GetTier(TEXT("Raj")), ERelationshipTier::Friend);
	TestEqual(TEXT("restored ZhangSir affinity"), Sys.GetAffinity(TEXT("ZhangSir")), 40);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
