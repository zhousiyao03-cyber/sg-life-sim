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
#include "Systems/CareerSubsystem.h"
#include "Systems/CareerTypes.h"
#include "Systems/EconomicEventSubsystem.h"
#include "Systems/EndingSubsystem.h"
#include "Systems/MilestoneSubsystem.h"
#include "Systems/MilestoneSystem.h"
#include "Systems/HorrorEventSubsystem.h"
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

	// 第一人称：身体随控制器（鼠标）水平朝向转；俯仰只转相机不转身体。
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		// 第一人称下身体跟视线，不再朝移动方向转。
		MoveComp->bOrientRotationToMovement = false;
	}

	// 相机挂在胶囊眼高，自己吃控制器俯仰/偏航（鼠标视角）。
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 64.f)); // 眼高
	FirstPersonCamera->bUsePawnControlRotation = true;

	// 第一人称看不到自己那具（占位）身体；mesh 仍在（投影/未来镜面用），只是 owner 不可见。
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetOwnerNoSee(true);
	}
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
		if (UEconomicEventSubsystem* Events = GI->GetSubsystem<UEconomicEventSubsystem>())
		{
			Events->OnEconomicEvent.AddUniqueDynamic(this, &ASGPlayerCharacter::HandleEconomicEvent);
		}
		if (UMilestoneSubsystem* Milestones = GI->GetSubsystem<UMilestoneSubsystem>())
		{
			Milestones->OnMilestoneCompleted.AddUniqueDynamic(this, &ASGPlayerCharacter::HandleMilestoneCompleted);
		}
		if (UHorrorEventSubsystem* Horror = GI->GetSubsystem<UHorrorEventSubsystem>())
		{
			Horror->OnHorrorEvent.AddUniqueDynamic(this, &ASGPlayerCharacter::HandleHorrorEvent);
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

		// 第一人称鼠标视角：legacy 轴 Turn/LookUp（见 DefaultInput.ini）→ 控制器偏航/俯仰。
		// EnhancedInputComponent 继承自 UInputComponent，legacy BindAxis 仍可用。
		PlayerInputComponent->BindAxis(TEXT("Turn"), this, &APawn::AddControllerYawInput);
		PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &APawn::AddControllerPitchInput);

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

void ASGPlayerCharacter::HandleEconomicEvent(FText Title)
{
	if (HudWidget && !Title.IsEmpty())
	{
		HudWidget->ShowAchievementToast(FText::FromString(
			FString::Printf(TEXT("📰 %s"), *Title.ToString())), 5.f);
	}
}

void ASGPlayerCharacter::HandleMilestoneCompleted(EMilestone Milestone)
{
	if (HudWidget)
	{
		HudWidget->ShowAchievementToast(FText::FromString(FString::Printf(
			TEXT("🎉 里程碑达成：%s"), *FMilestoneSystem::GetTitle(Milestone).ToString())), 6.f);
	}
}

void ASGPlayerCharacter::HandleHorrorEvent(FText Title)
{
	// 用对话气泡（底部居中、停留久一点）呈现阴森文案，比顶部 toast 更沉浸。
	if (HudWidget && !Title.IsEmpty())
	{
		HudWidget->SetDialogueText(FText::FromString(FString::Printf(TEXT("🕯 %s"), *Title.ToString())));
		FTimerDelegate ClearDel = FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (HudWidget) { HudWidget->SetDialogueText(FText::GetEmpty()); }
		});
		GetWorldTimerManager().SetTimer(DialogueClearTimer, ClearDel, 8.f, /*bLoop=*/false);
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

	// 钱包行（现金 + 净资产，有按揭时追加房贷余额）
	if (UEconomySubsystem* Eco = GI ? GI->GetSubsystem<UEconomySubsystem>() : nullptr)
	{
		FString Wallet = FString::Printf(TEXT("现金 %s   ·   净资产 %s"),
			*FormatMoney(Eco->GetBalance(ECurrencyAccount::Cash)),
			*FormatMoney(Eco->GetNetWorth()));
		if (UAssetsSubsystem* Assets = GI ? GI->GetSubsystem<UAssetsSubsystem>() : nullptr)
		{
			if (Assets->HasMortgage())
			{
				Wallet += FString::Printf(TEXT("   ·   房贷 %s（月供 %s）"),
					*FormatMoney(Assets->GetMortgageBalance()),
					*FormatMoney(Assets->GetMortgageMonthlyPayment()));
			}
		}
		HudWidget->SetWalletText(FText::FromString(Wallet));
	}

	// 属性行（职位 + 月薪 · 能量 / 心情 / 健康）
	if (UPlayerStateSubsystem* PS = GI ? GI->GetSubsystem<UPlayerStateSubsystem>() : nullptr)
	{
		FString Stats;
		if (UCareerSubsystem* Career = GI ? GI->GetSubsystem<UCareerSubsystem>() : nullptr)
		{
			Stats = FString::Printf(TEXT("%s（月薪 %s）   ·   "),
				*UEnum::GetDisplayValueAsText(Career->GetLevel()).ToString(),
				*FormatMoney(Career->GetGrossSalaryCents()));
		}
		Stats += FString::Printf(TEXT("能量 %d · 心情 %d · 健康 %d · 专业 %d"),
			PS->GetAttribute(EPlayerAttribute::Energy),
			PS->GetAttribute(EPlayerAttribute::Mood),
			PS->GetAttribute(EPlayerAttribute::Health),
			PS->GetAttribute(EPlayerAttribute::Professional));
		HudWidget->SetStatsText(FText::FromString(Stats));
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
			FString Progression = FString::Printf(TEXT("身份 %s · 住房 %s · 走向 %s"),
				*StatusText.ToString(), *HousingText.ToString(), *LeaningText.ToString());
			// 农历七月（鬼月）期间在进阶行末尾挂个阴森指示。
			if (UHorrorEventSubsystem* Horror = GI->GetSubsystem<UHorrorEventSubsystem>())
			{
				if (Horror->IsGhostMonth())
				{
					Progression += TEXT("   ·   🕯 农历七月");
				}
			}
			HudWidget->SetProgressionText(FText::FromString(Progression));
		}
	}

	// 目标行（主线方向 + 进度）
	if (UMilestoneSubsystem* Milestones = GI ? GI->GetSubsystem<UMilestoneSubsystem>() : nullptr)
	{
		HudWidget->SetObjectiveText(Milestones->GetActiveObjectiveText());
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
	const AController* Ctrl = GetController();
	if (!Ctrl)
	{
		return;
	}

	// 第一人称：相对控制器（视线）水平朝向移动。Axis.Y 前后，Axis.X 左右。
	const FRotator YawRotation(0.f, Ctrl->GetControlRotation().Yaw, 0.f);
	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(Forward, Axis.Y);
	AddMovementInput(Right, Axis.X);
}
