#include "Characters/SGInteractableNPC.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"

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
	// Task 9 起接入 UMG 对话框；原型先打日志验证交互链路打通
	UE_LOG(LogTemp, Log, TEXT("[NPC] %s: %s"),
		*SpeakerName.ToString(), *DialogueLine.ToString());
}

FText ASGInteractableNPC::GetInteractionPrompt_Implementation() const
{
	return FText::FromString(TEXT("[E] 对话"));
}
