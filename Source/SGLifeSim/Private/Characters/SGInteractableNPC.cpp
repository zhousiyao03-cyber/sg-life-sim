#include "Characters/SGInteractableNPC.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"

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
	// 原型：用屏幕文本显示对话（替代 UMG 对话框，受当前工具链限制）。
	UE_LOG(LogTemp, Log, TEXT("[NPC] %s: %s"),
		*SpeakerName.ToString(), *DialogueLine.ToString());
	if (GEngine)
	{
		const FString Line = FString::Printf(TEXT("%s：%s"),
			*SpeakerName.ToString(), *DialogueLine.ToString());
		// Key=2 固定槽，重复交互刷新同一行；显示 5 秒
		GEngine->AddOnScreenDebugMessage(2, 5.f, FColor::Yellow, Line);
	}
}

FText ASGInteractableNPC::GetInteractionPrompt_Implementation() const
{
	return FText::FromString(TEXT("[E] 对话"));
}
