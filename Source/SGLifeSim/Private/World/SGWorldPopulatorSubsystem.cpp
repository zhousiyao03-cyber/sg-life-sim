#include "World/SGWorldPopulatorSubsystem.h"

#include "Characters/SGInteractableNPC.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectGlobals.h"

bool USGWorldPopulatorSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

TArray<FNpcSpawnSpec> USGWorldPopulatorSubsystem::GetRosterForLevel(const FString& LevelName)
{
	TArray<FNpcSpawnSpec> Roster;

	if (LevelName.Contains(TEXT("L_Rental")))
	{
		// 出租屋：手摆的 Ah Hua 已在场（幂等会跳过）；代码补一个楼下保安 Uncle Lim。
		Roster.Emplace(
			TEXT("UncleLim"),
			FText::FromString(TEXT("保安 Uncle Lim")),
			TEXT("回来啦？今天加班到这么晚，辛苦咯。"),
			FVector(350.f, -250.f, 0.f), -120.f);
	}
	else if (LevelName.Contains(TEXT("L_HawkerCenter")))
	{
		// 食阁：卖鸡饭的 Ah Mei 阿姨 + 来吃午饭的同事 Wei。
		Roster.Emplace(
			TEXT("AhMei"),
			FText::FromString(TEXT("食阁阿姨 Ah Mei")),
			TEXT("小伙子，吃什么？今天鸡饭特价。"),
			FVector(300.f, 200.f, 0.f), 200.f);
		Roster.Emplace(
			TEXT("Wei"),
			FText::FromString(TEXT("同事 Wei")),
			TEXT("哟，也来这边吃啊？坐坐坐。"),
			FVector(-280.f, 260.f, 0.f), 90.f);
	}

	return Roster;
}

void USGWorldPopulatorSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	const FString LevelName = UWorld::RemovePIEPrefix(InWorld.GetMapName());
	const TArray<FNpcSpawnSpec> Roster = GetRosterForLevel(LevelName);
	if (Roster.Num() == 0)
	{
		return;
	}

	// 锚点：用第一个 PlayerStart 的位置（保证在可达地面上）；没有就退回原点。
	FVector Anchor = FVector::ZeroVector;
	{
		TArray<AActor*> Starts;
		UGameplayStatics::GetAllActorsOfClass(&InWorld, APlayerStart::StaticClass(), Starts);
		if (Starts.Num() > 0 && Starts[0])
		{
			Anchor = Starts[0]->GetActorLocation();
		}
	}

	// 幂等：收集已存在的 NpcId（手摆的 Ah Hua 等），避免重复生成。
	TSet<FName> ExistingIds;
	{
		TArray<AActor*> Existing;
		UGameplayStatics::GetAllActorsOfClass(&InWorld, ASGInteractableNPC::StaticClass(), Existing);
		for (AActor* A : Existing)
		{
			if (const ASGInteractableNPC* Npc = Cast<ASGInteractableNPC>(A))
			{
				ExistingIds.Add(Npc->GetNpcId());
			}
		}
	}

	// 复用主角骨骼网格 + idle 动画作占位外观（视觉分化留待后续美术）。
	USkeletalMesh* NpcMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Characters/Player/SK_Player.SK_Player"));
	UAnimSequence* IdleAnim = LoadObject<UAnimSequence>(nullptr, TEXT("/Game/Characters/Player/Animations/A_Idle.A_Idle"));

	for (const FNpcSpawnSpec& Spec : Roster)
	{
		if (ExistingIds.Contains(Spec.NpcId))
		{
			continue;
		}

		const FVector Loc = Anchor + Spec.OffsetFromAnchor;
		const FRotator Rot(0.f, Spec.YawDegrees, 0.f);

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		ASGInteractableNPC* Npc = InWorld.SpawnActor<ASGInteractableNPC>(
			ASGInteractableNPC::StaticClass(), Loc, Rot, Params);
		if (!Npc)
		{
			continue;
		}

		Npc->ConfigureNpc(Spec.NpcId, Spec.SpeakerName, Spec.DialogueLine, NpcMesh);

		if (IdleAnim)
		{
			if (USkeletalMeshComponent* MeshComp = Npc->FindComponentByClass<USkeletalMeshComponent>())
			{
				MeshComp->PlayAnimation(IdleAnim, /*bLooping=*/true);
			}
		}

		ExistingIds.Add(Spec.NpcId);
	}
}
