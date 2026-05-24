#include "Characters/SGPlayerCharacter.h"

#include "Animation/AnimSequence.h"
#include "Camera/CameraComponent.h"
#include "Characters/SGInteractableNPC.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "Interactables/InteractableInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Systems/EconomySubsystem.h"
#include "Systems/EconomyTypes.h"
#include "Systems/PlayerStateSubsystem.h"
#include "Systems/PlayerStatsTypes.h"
#include "Systems/ProgressSubsystem.h"
#include "Systems/DialogueSubsystem.h"
#include "Systems/RelationshipSubsystem.h"
#include "Systems/RelationshipTypes.h"
#include "Systems/ResidencySubsystem.h"
#include "Systems/AssetsSubsystem.h"
#include "Systems/EndingSubsystem.h"
#include "Systems/TimeBlock.h"
#include "Systems/TimeSubsystem.h"
#include "TimerManager.h"
#include "UI/SGDialogueWidget.h"
#include "UI/SGHudWidget.h"
#include "UI/SGLocationMenuWidget.h"

namespace
{
	// 把「分」格式化成「$X.XX」。
	FString FormatMoney(int64 Cents)
	{
		const TCHAR* Sign = (Cents < 0) ? TEXT("-") : TEXT("");
		const int64 Abs = FMath::Abs(Cents);
		return FString::Printf(TEXT("%s$%lld.%02lld"), Sign, Abs / 100, Abs % 100);
	}
}

ASGPlayerCharacter::ASGPlayerCharacter()
{
	// 单节点 locomotion 需要每帧检查速度来切换 Idle/Walk
	PrimaryActorTick.bCanEverTick = true;

	// 等距固定视角：角色朝移动方向转，不跟随控制器朝向
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->RotationRate = FRotator(0.f, 540.f, 0.f);
	}

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 1200.f;
	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;
	CameraBoom->SetRelativeRotation(FRotator(-45.f, -45.f, 0.f));
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 10.f;

	IsometricCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("IsometricCamera"));
	IsometricCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	IsometricCamera->ProjectionMode = ECameraProjectionMode::Orthographic;
	IsometricCamera->OrthoWidth = 700.f;
	IsometricCamera->bUsePawnControlRotation = false;
}

void ASGPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
					LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (DefaultMappingContext)
				{
					Subsystem->AddMappingContext(DefaultMappingContext, 0);
				}
			}
		}

		// 原型 HUD：纯 C++ UMG，无需 BP widget 资产
		HudWidget = CreateWidget<USGHudWidget>(PC, USGHudWidget::StaticClass());
		if (HudWidget)
		{
			HudWidget->AddToViewport();
		}
	}

	// 订阅成就解锁 → HUD toast。Director / Progress 在 GameInstance 层跨关卡保留，
	// 玩家与 HUD 每次切关卡重建，这里每次 BeginPlay 重新绑定。
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UProgressSubsystem* Prog = GI->GetSubsystem<UProgressSubsystem>())
		{
			Prog->OnAchievementUnlocked.AddUniqueDynamic(this, &ASGPlayerCharacter::HandleAchievementUnlocked);
		}
	}

	// 起步先站立
	UpdateLocomotionAnimation();
}

void ASGPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateLocomotionAnimation();
	DrawPrototypeHUD();
}

void ASGPlayerCharacter::UpdateLocomotionAnimation()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		return;
	}

	// 动画引用正路是 BP 子类赋值；但若为空（如 Live Coding 未刷新反射、
	// 新 UPROPERTY 尚未生效），用固定资产路径兜底加载，保证 demo 能跑。
	UAnimSequence* Idle = IdleAnim;
	UAnimSequence* Walk = WalkAnim;
	if (!Idle)
	{
		Idle = LoadObject<UAnimSequence>(nullptr, TEXT("/Game/Characters/Player/Animations/A_Idle.A_Idle"));
	}
	if (!Walk)
	{
		Walk = LoadObject<UAnimSequence>(nullptr, TEXT("/Game/Characters/Player/Animations/A_Walk.A_Walk"));
	}

	// 只看水平速度，竖直分量（落地/跳跃）不算「走」
	const float HorizontalSpeed = GetVelocity().Size2D();
	const float Threshold = (WalkSpeedThreshold > 0.f) ? WalkSpeedThreshold : 10.f;
	UAnimSequence* DesiredAnim = (HorizontalSpeed > Threshold) ? Walk : Idle;

	if (DesiredAnim && DesiredAnim != CurrentAnim)
	{
		MeshComp->PlayAnimation(DesiredAnim, /*bLooping=*/true);
		CurrentAnim = DesiredAnim;
	}
}

void ASGPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASGPlayerCharacter::Move);
		}

		// E / T / M：原型阶段动作引用尚未做成 UPROPERTY（避免热编译反射问题），
		// 直接按固定路径加载并绑定。BeginPlay 已经把 IMC_Default 加进 Enhanced Input。
		auto BindIA = [EIC, this](const TCHAR* Path, void (ASGPlayerCharacter::*Fn)())
		{
			if (UInputAction* IA = LoadObject<UInputAction>(nullptr, Path))
			{
				EIC->BindAction(IA, ETriggerEvent::Started, this, Fn);
			}
		};
		BindIA(TEXT("/Game/Input/IA_Interact.IA_Interact"), &ASGPlayerCharacter::TryInteract);
		BindIA(TEXT("/Game/Input/IA_AdvanceTime.IA_AdvanceTime"), &ASGPlayerCharacter::AdvanceTime);
		BindIA(TEXT("/Game/Input/IA_OpenLocationMenu.IA_OpenLocationMenu"), &ASGPlayerCharacter::SwitchLocation);
	}
}

AActor* ASGPlayerCharacter::FindNearbyInteractable() const
{
	TArray<AActor*> Npcs;
	UGameplayStatics::GetAllActorsOfClass(this, ASGInteractableNPC::StaticClass(), Npcs);

	AActor* Nearest = nullptr;
	float NearestDistSq = FMath::Square(300.f); // 交互距离 3m
	const FVector MyLoc = GetActorLocation();
	for (AActor* Npc : Npcs)
	{
		const float DistSq = FVector::DistSquared(MyLoc, Npc->GetActorLocation());
		if (DistSq < NearestDistSq)
		{
			NearestDistSq = DistSq;
			Nearest = Npc;
		}
	}
	return Nearest;
}

void ASGPlayerCharacter::TryInteract()
{
	AActor* Nearest = FindNearbyInteractable();
	if (!Nearest || !Nearest->Implements<UInteractableInterface>())
	{
		return;
	}

	IInteractableInterface::Execute_OnInteract(Nearest, this);

	const ASGInteractableNPC* Npc = Cast<ASGInteractableNPC>(Nearest);
	if (!Npc)
	{
		return;
	}

	UGameInstance* GI = GetGameInstance();
	APlayerController* PC = Cast<APlayerController>(GetController());

	// 交谈消耗能量（spec §5/§6）。好感不再在交互瞬间平加，改由对话选项效果决定。
	if (UPlayerStateSubsystem* PS = GI ? GI->GetSubsystem<UPlayerStateSubsystem>() : nullptr)
	{
		PS->ModifyAttribute(EPlayerAttribute::Energy, -5);
	}

	// 若该 NPC 有注册的对话树 → 弹出对话界面（数据驱动，选项门控 + 效果）。
	UDialogueSubsystem* Dialogue = GI ? GI->GetSubsystem<UDialogueSubsystem>() : nullptr;
	if (Dialogue && PC)
	{
		if (!DialogueWidget)
		{
			DialogueWidget = CreateWidget<USGDialogueWidget>(PC, USGDialogueWidget::StaticClass());
		}
		if (DialogueWidget && DialogueWidget->OpenForTree(Npc->GetNpcId()))
		{
			return; // 成功开对话，台词/选项由对话界面接管
		}
	}

	// 兜底：没有对话树（或开启失败）时，仍在 HUD 气泡显示一句台词，5 秒后消失。
	if (HudWidget)
	{
		HudWidget->SetDialogueText(Npc->GetDialogueDisplayText());
		FTimerDelegate ClearDel = FTimerDelegate::CreateLambda([this]()
		{
			if (HudWidget)
			{
				HudWidget->SetDialogueText(FText::GetEmpty());
			}
		});
		GetWorldTimerManager().SetTimer(DialogueClearTimer, ClearDel, 5.f, /*bLoop=*/false);
	}
}

void ASGPlayerCharacter::HandleAchievementUnlocked(FName AchievementId)
{
	if (HudWidget)
	{
		HudWidget->ShowAchievementToast(FText::FromString(
			FString::Printf(TEXT("🏆 成就解锁：%s"), *AchievementId.ToString())));
	}
}

void ASGPlayerCharacter::AdvanceTime()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UTimeSubsystem* TimeSys = GI->GetSubsystem<UTimeSubsystem>())
		{
			TimeSys->AdvanceBlock();
		}
	}
}

void ASGPlayerCharacter::SwitchLocation()
{
	// M 键开/关地点菜单（USGLocationMenuWidget）。点菜单里的按钮才真正 OpenLevel。
	// TimeSubsystem 在 GameInstance 上，跨关卡保留 —— 验证 ADR 0005。
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}

	if (LocationMenu && LocationMenu->IsInViewport())
	{
		LocationMenu->CloseMenu();
		return;
	}

	if (!LocationMenu)
	{
		LocationMenu = CreateWidget<USGLocationMenuWidget>(PC, USGLocationMenuWidget::StaticClass());
	}
	if (LocationMenu)
	{
		LocationMenu->OpenMenu();
	}
}

void ASGPlayerCharacter::DrawPrototypeHUD()
{
	if (!HudWidget)
	{
		return;
	}

	UGameInstance* GI = GetGameInstance();
	if (UTimeSubsystem* TimeSys = GI ? GI->GetSubsystem<UTimeSubsystem>() : nullptr)
	{
		const FText BlockText = UEnum::GetDisplayValueAsText(TimeSys->GetCurrentBlock());
		const FText WeekdayText = UEnum::GetDisplayValueAsText(TimeSys->GetWeekday());
		HudWidget->SetStatusText(FText::FromString(FString::Printf(
			TEXT("Day %d · %s · %s    [E] 交谈  [T] 推进时间  [M] 菜单"),
			TimeSys->GetDayNumber(), *WeekdayText.ToString(), *BlockText.ToString())));
	}

	// 钱包行（现金 + 净资产）
	if (UEconomySubsystem* Eco = GI ? GI->GetSubsystem<UEconomySubsystem>() : nullptr)
	{
		HudWidget->SetWalletText(FText::FromString(FString::Printf(
			TEXT("现金 %s   ·   净资产 %s"),
			*FormatMoney(Eco->GetBalance(ECurrencyAccount::Cash)),
			*FormatMoney(Eco->GetNetWorth()))));
	}

	// 属性行（能量 / 心情 / 健康）
	if (UPlayerStateSubsystem* PS = GI ? GI->GetSubsystem<UPlayerStateSubsystem>() : nullptr)
	{
		HudWidget->SetStatsText(FText::FromString(FString::Printf(
			TEXT("能量 %d · 心情 %d · 健康 %d"),
			PS->GetAttribute(EPlayerAttribute::Energy),
			PS->GetAttribute(EPlayerAttribute::Mood),
			PS->GetAttribute(EPlayerAttribute::Health))));
	}

	// 进阶行（身份 · 房产 · 终局倾向）
	{
		UResidencySubsystem* Res = GI ? GI->GetSubsystem<UResidencySubsystem>() : nullptr;
		UAssetsSubsystem* Assets = GI ? GI->GetSubsystem<UAssetsSubsystem>() : nullptr;
		UEndingSubsystem* End = GI ? GI->GetSubsystem<UEndingSubsystem>() : nullptr;
		if (Res && Assets && End)
		{
			const FText StatusText = UEnum::GetDisplayValueAsText(Res->GetStatus());
			const FText HousingText = UEnum::GetDisplayValueAsText(Assets->GetHousingTier());
			const FText LeaningText = UEnum::GetDisplayValueAsText(End->GetCurrentLeaning());
			HudWidget->SetProgressionText(FText::FromString(FString::Printf(
				TEXT("身份 %s · 住房 %s · 走向 %s"),
				*StatusText.ToString(), *HousingText.ToString(), *LeaningText.ToString())));
		}
	}

	// 靠近可交互对象时显示其提示（如「[E] 对话」），否则隐藏
	AActor* Nearby = FindNearbyInteractable();
	if (Nearby && Nearby->Implements<UInteractableInterface>())
	{
		HudWidget->SetPromptText(IInteractableInterface::Execute_GetInteractionPrompt(Nearby));
	}
	else
	{
		HudWidget->SetPromptText(FText::GetEmpty());
	}
}

void ASGPlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	// 等距视角下，2D 输入直接映射到世界 X/Y 平面：X 左右，Y 前后
	AddMovementInput(FVector(1.f, 0.f, 0.f), Axis.X);
	AddMovementInput(FVector(0.f, 1.f, 0.f), Axis.Y);
}
