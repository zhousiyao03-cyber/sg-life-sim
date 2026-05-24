#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#include "World/SGWorldPopulatorSubsystem.h"
#include "Systems/SGDialogueContent.h"
#include "Systems/SGDialogueTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * 填充世界的每个 NPC 都必须有注册的对话树 —— 否则玩家走近只会得到兜底气泡。Plan 12。
 * 这条把「世界内容」与「对话内容」钉在一起，防止以后加 NPC 忘了配树。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWorldPopulatorRosterTest,
	"SGLifeSim.World.RosterNpcsHaveDialogueTrees",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWorldPopulatorRosterTest::RunTest(const FString& Parameters)
{
	// 收集所有已注册对话树的 id。
	TSet<FName> TreeIds;
	for (const FDialogueTree& Tree : SGDialogueContent::BuildAllTrees())
	{
		TreeIds.Add(Tree.TreeId);
	}

	const TCHAR* Levels[] = { TEXT("L_Rental"), TEXT("L_HawkerCenter"), TEXT("L_Office") };
	int32 TotalSpawned = 0;
	for (const TCHAR* Level : Levels)
	{
		const TArray<FNpcSpawnSpec> Roster = USGWorldPopulatorSubsystem::GetRosterForLevel(Level);
		for (const FNpcSpawnSpec& Spec : Roster)
		{
			++TotalSpawned;
			TestTrue(
				FString::Printf(TEXT("NPC '%s' (in %s) has a registered dialogue tree"),
					*Spec.NpcId.ToString(), Level),
				TreeIds.Contains(Spec.NpcId));
		}
	}

	// 至少要真的有内容（防止 roster 被清空后测试空跑也算过）。
	TestTrue(TEXT("roster populates at least 3 NPCs across levels"), TotalSpawned >= 3);

	// 关卡 roster 的具体成员。
	const TArray<FNpcSpawnSpec> Hawker = USGWorldPopulatorSubsystem::GetRosterForLevel(TEXT("L_HawkerCenter"));
	const bool bHasAhMei = Hawker.ContainsByPredicate([](const FNpcSpawnSpec& S){ return S.NpcId == TEXT("AhMei"); });
	const bool bHasWei   = Hawker.ContainsByPredicate([](const FNpcSpawnSpec& S){ return S.NpcId == TEXT("Wei"); });
	TestTrue(TEXT("hawker center includes Ah Mei"), bHasAhMei);
	TestTrue(TEXT("hawker center includes colleague Wei"), bHasWei);

	const TArray<FNpcSpawnSpec> Rental = USGWorldPopulatorSubsystem::GetRosterForLevel(TEXT("L_Rental"));
	const bool bHasUncle = Rental.ContainsByPredicate([](const FNpcSpawnSpec& S){ return S.NpcId == TEXT("UncleLim"); });
	TestTrue(TEXT("rental block includes Uncle Lim"), bHasUncle);

	const TArray<FNpcSpawnSpec> Office = USGWorldPopulatorSubsystem::GetRosterForLevel(TEXT("L_Office"));
	const bool bHasTan = Office.ContainsByPredicate([](const FNpcSpawnSpec& S){ return S.NpcId == TEXT("ManagerTan"); });
	TestTrue(TEXT("office includes Manager Tan"), bHasTan);

	// PIE 前缀也要能匹配。
	const TArray<FNpcSpawnSpec> Pie = USGWorldPopulatorSubsystem::GetRosterForLevel(TEXT("UEDPIE_0_L_HawkerCenter"));
	TestEqual(TEXT("PIE-prefixed level name still resolves roster"), Pie.Num(), Hawker.Num());

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
