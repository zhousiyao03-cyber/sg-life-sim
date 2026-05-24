#include "Characters/SGInteractableNPC.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Engine/SkeletalMesh.h"

ASGInteractableNPC::ASGInteractableNPC()
{
	PrimaryActorTick.bCanEverTick = false;

	NpcMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("NpcMesh"));
	RootComponent = NpcMesh;

	InteractionRange = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionRange"));
	InteractionRange->SetupAttachment(RootComponent);
	InteractionRange->SetSphereRadius(200.f);
	InteractionRange->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	SpeakerName = FText::FromString(TEXT("邻居 Ah Hua"));
	DialogueLine = FText::FromString(TEXT("你怎么搬来这种小破组屋了？刚来吧？"));
}

void ASGInteractableNPC::OnInteract_Implementation(AActor* Interactor)
{
	// 交互事件本身在这里记日志（gameplay 钩子）；台词的显示交给玩家的 HUD
	// 对话气泡（ASGPlayerCharacter 持有 UMG widget），见 GetDialogueDisplayText。
	UE_LOG(LogTemp, Log, TEXT("[NPC] %s: %s"),
		*SpeakerName.ToString(), *DialogueLine.ToString());
}

FText ASGInteractableNPC::GetInteractionPrompt_Implementation() const
{
	return FText::FromString(TEXT("[E] 对话"));
}

FText ASGInteractableNPC::GetDialogueDisplayText() const
{
	return FText::FromString(FString::Printf(TEXT("%s：%s"),
		*SpeakerName.ToString(), *DialogueLine.ToString()));
}

void ASGInteractableNPC::ConfigureNpc(FName InNpcId, const FText& InSpeakerName,
	const FString& InDialogueLine, USkeletalMesh* InMesh)
{
	NpcId = InNpcId;
	SpeakerName = InSpeakerName;
	DialogueLine = FText::FromString(InDialogueLine);

	if (InMesh && NpcMesh)
	{
		NpcMesh->SetSkeletalMesh(InMesh);
	}
}
