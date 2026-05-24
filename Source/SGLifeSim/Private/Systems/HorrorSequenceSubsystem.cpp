#include "Systems/HorrorSequenceSubsystem.h"
#include "Systems/HorrorSceneRegistry.h"
#include "Systems/SanitySubsystem.h"
#include "Systems/HorrorCodexSubsystem.h"

#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

bool UHorrorSequenceSubsystem::EnterScene(EHorrorScene Scene)
{
	// 防重入：演出途中再触发不叠。
	if (bInScene)
	{
		return false;
	}

	const FHorrorSceneDef Def = FHorrorSceneRegistry::GetSceneDef(Scene);
	if (Def.LevelName.IsNone())
	{
		return false; // None / 无效场景
	}

	UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	if (!World)
	{
		return false;
	}

	// 记下来源关卡 + 玩家坐标（第一版先存，回程暂不强制还原，重生 PlayerStart）。
	ReturnLevelName = FName(*World->GetName());
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			ReturnLocation = Pawn->GetActorLocation();
			ReturnRotation = Pawn->GetActorRotation();
		}
	}

	ActiveScene = Scene;
	bInScene = true;

	UGameplayStatics::OpenLevel(World, Def.LevelName);
	return true;
}

void UHorrorSequenceSubsystem::ExitScene()
{
	if (!bInScene)
	{
		return;
	}

	const EHorrorScene Scene = ActiveScene;
	const FName Return = ReturnLevelName;

	// 结算（扣理智 + 记图鉴 + 广播事后文案）。
	ResolveOutcome(Scene);

	// 复位状态，回来源关卡。
	bInScene = false;
	ActiveScene = EHorrorScene::None;

	if (!Return.IsNone())
	{
		if (UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr)
		{
			UGameplayStatics::OpenLevel(World, Return);
		}
	}
}

FHorrorSceneDef UHorrorSequenceSubsystem::ResolveOutcome(EHorrorScene Scene)
{
	const FHorrorSceneDef Def = FHorrorSceneRegistry::GetSceneDef(Scene);

	if (UGameInstance* GI = GetGameInstance())
	{
		if (Def.SanityCost > 0)
		{
			if (USanitySubsystem* San = GI->GetSubsystem<USanitySubsystem>())
			{
				San->Drain(Def.SanityCost);
			}
		}
		if (Def.CodexEntry != EHorrorEvent::None)
		{
			if (UHorrorCodexSubsystem* Codex = GI->GetSubsystem<UHorrorCodexSubsystem>())
			{
				Codex->RecordEncounter(Def.CodexEntry);
			}
		}
	}

	if (!Def.AftermathText.IsEmpty())
	{
		OnAftermath.Broadcast(Def.AftermathText);
	}
	return Def;
}
