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
#include "Systems/EndingTypes.h"
#include "Systems/MilestoneSubsystem.h"
#include "Systems/MilestoneSystem.h"
#include "Systems/HorrorEventSubsystem.h"
#include "Systems/HorrorCodexSubsystem.h"
#include "Systems/NightCommuteSubsystem.h"
#include "Systems/PlayerVitalsSubsystem.h"
#include "Systems/WeaponSubsystem.h"
#include "Systems/WeaponTypes.h"
#include "Systems/WantedSubsystem.h"
#include "Systems/SGAudioSubsystem.h"
#include "Systems/WeatherSubsystem.h"
#include "Systems/WeatherTypes.h"
#include "World/LocationManagerSubsystem.h"
#include "World/SGStreetNPC.h"
#include "Systems/SanitySubsystem.h"
#include "Systems/TimeBlock.h"
#include "Systems/TimeSubsystem.h"
#include "TimerManager.h"
#include "UI/SGDialogueWidget.h"
#include "UI/SGEndingWidget.h"
#include "UI/SGHudWidget.h"
#include "UI/SGLocationMenuWidget.h"
#include "UI/SGMinimapWidget.h"
#include "World/LocationRegistry.h"

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

		// 冲刺/蹲（D 块）：常速取移动组件默认，冲刺为其 1.7 倍；允许下蹲。
		NormalWalkSpeed = MoveComp->MaxWalkSpeed;
		SprintWalkSpeed = NormalWalkSpeed * 1.7f;
		MoveComp->NavAgentProps.bCanCrouch = true;
		MoveComp->MaxWalkSpeedCrouched = NormalWalkSpeed * 0.5f;
	}

	// 相机挂在胶囊眼高，自己吃控制器俯仰/偏航（鼠标视角）。
	// 略向前偏，让相机出在「脸前」而非头部内部——低头能看见自己的躯干/手脚而不穿到头里。
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(10.f, 0.f, 64.f)); // 眼高 + 略前
	FirstPersonCamera->bUsePawnControlRotation = true;

	// 「能看到自己身体」的第一人称：不再对 owner 隐藏 mesh，低头能看见手脚躯干。
	// 注意：占位 SK_Player 是整具人形，相机在眼高会看到颈部内侧——故相机前移（上面），
	// 后续换专用 FP 手臂 mesh 或给身体 mesh 隐藏头部骨骼可进一步消除穿模。
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

		// 城市枢纽关卡才显示小地图。
		const FString Level = UGameplayStatics::GetCurrentLevelName(this, /*bRemovePrefix=*/true);
		if (Level.Contains(FLocationRegistry::GetCityLevelName().ToString()))
		{
			MinimapWidget = CreateWidget<USGMinimapWidget>(PC, USGMinimapWidget::StaticClass());
			if (MinimapWidget)
			{
				MinimapWidget->AddToViewport();
			}
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
		if (UEndingSubsystem* Ending = GI->GetSubsystem<UEndingSubsystem>())
		{
			Ending->OnEndingChosen.AddUniqueDynamic(this, &ASGPlayerCharacter::HandleEndingChosen);
		}
		if (UNightCommuteSubsystem* NightCommute = GI->GetSubsystem<UNightCommuteSubsystem>())
		{
			NightCommute->OnResolved.AddUniqueDynamic(this, &ASGPlayerCharacter::HandleNightCommuteResolved);
		}
		if (UPlayerVitalsSubsystem* Vitals = GI->GetSubsystem<UPlayerVitalsSubsystem>())
		{
			Vitals->OnPlayerDied.AddUniqueDynamic(this, &ASGPlayerCharacter::HandlePlayerDied);
			Vitals->OnPlayerRespawned.AddUniqueDynamic(this, &ASGPlayerCharacter::HandlePlayerRespawned);
		}
	}

	// 从室内回到城市枢纽：把玩家从 PlayerStart 拉回离开时的建筑门口（只生效一次）。
	if (UGameInstance* GI = GetGameInstance())
	{
		if (USGLocationManagerSubsystem* Loc = GI->GetSubsystem<USGLocationManagerSubsystem>())
		{
			FVector ReturnLoc; FRotator ReturnRot;
			if (Loc->ConsumePendingReturn(ReturnLoc, ReturnRot))
			{
				SetActorLocation(ReturnLoc);
				if (AController* Ctrl = GetController())
				{
					Ctrl->SetControlRotation(ReturnRot);
				}
			}
		}
	}

	// 记录本关出生点（落点已最终确定）：死亡重生传送回此处。
	SpawnLocation = GetActorLocation();
	SpawnRotation = GetController() ? GetController()->GetControlRotation() : GetActorRotation();

	// 起步先站立
	UpdateLocomotionAnimation();
}

void ASGPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bClimbing) { TickClimb(); }
	UpdateLocomotionAnimation();
	UpdateFootsteps(DeltaSeconds);
	DrawPrototypeHUD();

	if (MinimapWidget)
	{
		MinimapWidget->UpdatePlayerDot(GetActorLocation());
	}
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

	// 占位走路动画 A_Walk（Mixamo 骨架）循环首尾帧不连续，bLooping 硬跳回首帧导致
	// 「走着走着抽一下/像后退」。引擎自带走路动画是 UE5 Manny 骨架，骨名不同套不上，
	// 重定向又是给占位资产做的工序——故占位阶段一律播 Idle（走路时滑步前进，不抽动）。
	// 等换上真角色 + 干净走路循环（或做了 IK Retarget）后，恢复下面注释掉的按速切换。
	//
	//   const float HorizontalSpeed = GetVelocity().Size2D();
	//   const float Threshold = (WalkSpeedThreshold > 0.f) ? WalkSpeedThreshold : 10.f;
	//   UAnimSequence* DesiredAnim = (HorizontalSpeed > Threshold) ? Walk : Idle;
	(void)Walk; // 暂不使用，避免 unused 警告；保留加载逻辑以便恢复
	UAnimSequence* DesiredAnim = Idle;

	if (DesiredAnim && DesiredAnim != CurrentAnim)
	{
		MeshComp->PlayAnimation(DesiredAnim, /*bLooping=*/true);
		CurrentAnim = DesiredAnim;
	}
}

void ASGPlayerCharacter::UpdateFootsteps(float DeltaSeconds)
{
	// 只有在地面行走时累计步距；攀爬/跳跃中不算。
	const UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (bClimbing || !MoveComp || !MoveComp->IsMovingOnGround())
	{
		return;
	}

	const float Speed = GetVelocity().Size2D();
	if (Speed < WalkSpeedThreshold) { return; }

	DistanceSinceLastStep += Speed * DeltaSeconds;

	// 步距：常速约 1.8m 一步；速度越快步频越高（除以速度比例自然成立）。
	const float StepDistance = bIsCrouched ? 120.f : 180.f;
	if (DistanceSinceLastStep >= StepDistance)
	{
		DistanceSinceLastStep = 0.f;
		if (UGameInstance* GI = GetGameInstance())
		{
			if (USGAudioSubsystem* Audio = GI->GetSubsystem<USGAudioSubsystem>())
			{
				Audio->PlayCueAtLocation(ESGSound::Footstep, GetActorLocation());
			}
		}
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

		// 跳跃：legacy 动作 Jump（空格，见 DefaultInput.ini）→ ACharacter 内置跳跃。
		// 重力/落地由 CharacterMovement 处理，无需额外代码。
		PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ACharacter::Jump);
		PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ACharacter::StopJumping);

		// 攀爬：legacy 动作 Climb（C）→ 贴墙时切入/退出攀爬模式。
		PlayerInputComponent->BindAction(TEXT("Climb"), IE_Pressed, this, &ASGPlayerCharacter::ToggleClimb);

		// 出拳：legacy 动作 Punch（F）→ 近战打前方 StreetNPC。
		PlayerInputComponent->BindAction(TEXT("Punch"), IE_Pressed, this, &ASGPlayerCharacter::Punch);

		// 开火 / 换弹：legacy 动作 Fire（鼠标左键）/ Reload（R）（C 块枪械）。
		PlayerInputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &ASGPlayerCharacter::Fire);
		PlayerInputComponent->BindAction(TEXT("Reload"), IE_Pressed, this, &ASGPlayerCharacter::ReloadWeapon);

		// 冲刺（Shift 按住）/ 蹲（Ctrl 切换）/ 视角切换（V）（D 块）。
		PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Pressed, this, &ASGPlayerCharacter::StartSprint);
		PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Released, this, &ASGPlayerCharacter::StopSprint);
		PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Pressed, this, &ASGPlayerCharacter::ToggleCrouch);
		PlayerInputComponent->BindAction(TEXT("ToggleView"), IE_Pressed, this, &ASGPlayerCharacter::ToggleView);

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
	// 按接口找：NPC 和城市建筑入口都实现 IInteractableInterface，统一发现。
	TArray<AActor*> Interactables;
	UGameplayStatics::GetAllActorsWithInterface(this, UInteractableInterface::StaticClass(), Interactables);

	AActor* Nearest = nullptr;
	float NearestDistSq = FMath::Square(300.f); // 交互距离 3m
	const FVector MyLoc = GetActorLocation();
	for (AActor* Actor : Interactables)
	{
		const float DistSq = FVector::DistSquared(MyLoc, Actor->GetActorLocation());
		if (DistSq < NearestDistSq)
		{
			NearestDistSq = DistSq;
			Nearest = Actor;
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

void ASGPlayerCharacter::HandleEndingChosen(EEnding Ending)
{
	if (Ending == EEnding::None)
	{
		return;
	}
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}
	if (!EndingWidget)
	{
		EndingWidget = CreateWidget<USGEndingWidget>(PC, USGEndingWidget::StaticClass());
	}
	if (EndingWidget)
	{
		EndingWidget->ShowEnding(Ending);
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

void ASGPlayerCharacter::HandleNightCommuteResolved(const FText& Message)
{
	// 夜归抉择的结算文案走和恐怖事件同一条气泡通道（底部居中、停留久）。
	if (HudWidget && !Message.IsEmpty())
	{
		HudWidget->SetDialogueText(FText::FromString(FString::Printf(TEXT("🛗 %s"), *Message.ToString())));
		FTimerDelegate ClearDel = FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (HudWidget) { HudWidget->SetDialogueText(FText::GetEmpty()); }
		});
		GetWorldTimerManager().SetTimer(DialogueClearTimer, ClearDel, 8.f, /*bLoop=*/false);
	}
}

void ASGPlayerCharacter::HandlePlayerDied()
{
	// GTA 式重生：传送回本关出生点（黑屏淡入等演出待美术）。
	// 攀爬态先退出，免得带着 MOVE_Flying 重生。
	if (bClimbing) { StopClimb(); }
	SetActorLocation(SpawnLocation);
	if (AController* Ctrl = GetController())
	{
		Ctrl->SetControlRotation(SpawnRotation);
	}
}

void ASGPlayerCharacter::HandlePlayerRespawned(int32 HospitalFeeCents)
{
	if (HudWidget)
	{
		const FString Msg = FString::Printf(
			TEXT("🏥 你在医院醒来…医药费 %s"), *FormatMoney(HospitalFeeCents));
		HudWidget->SetDialogueText(FText::FromString(Msg));
		FTimerDelegate ClearDel = FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (HudWidget) { HudWidget->SetDialogueText(FText::GetEmpty()); }
		});
		GetWorldTimerManager().SetTimer(DialogueClearTimer, ClearDel, 6.f, /*bLoop=*/false);
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

		// 天气（H 块）。
		FString WeatherStr;
		if (UWeatherSubsystem* Weather = GI->GetSubsystem<UWeatherSubsystem>())
		{
			WeatherStr = FString::Printf(TEXT(" · %s"),
				*UEnum::GetDisplayValueAsText(Weather->GetWeather()).ToString());
		}

		HudWidget->SetStatusText(FText::FromString(FString::Printf(
			TEXT("Day %d · %s · %s%s    [E] 交谈  [T] 推进时间  [M] 菜单"),
			TimeSys->GetDayNumber(), *WeekdayText.ToString(), *BlockText.ToString(), *WeatherStr)));
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
		// 理智（恐怖玩法核心）：值 + 状态档
		if (USanitySubsystem* Sanity = GI->GetSubsystem<USanitySubsystem>())
		{
			Stats += FString::Printf(TEXT(" · 理智 %d（%s）"),
				Sanity->GetSanity(),
				*UEnum::GetDisplayValueAsText(Sanity->GetState()).ToString());
		}
		// 血量（B 块 GTA）：满血不打扰，受伤才用 ❤ 醒目提示。
		if (UPlayerVitalsSubsystem* Vitals = GI->GetSubsystem<UPlayerVitalsSubsystem>())
		{
			if (Vitals->GetHealth() < Vitals->GetMaxHealth())
			{
				Stats += FString::Printf(TEXT("  ·  ❤ %d/%d"),
					Vitals->GetHealth(), Vitals->GetMaxHealth());
			}
		}
		// 武器/弹药（C 块 GTA）：持枪才显示「🔫 名 弹/匣」。
		if (UWeaponSubsystem* Weapon = GI->GetSubsystem<UWeaponSubsystem>())
		{
			if (Weapon->HasGun())
			{
				Stats += FString::Printf(TEXT("  ·  🔫 %s %d/%d"),
					*UEnum::GetDisplayValueAsText(Weapon->GetWeapon()).ToString(),
					Weapon->GetAmmoInMag(), Weapon->GetMagSize());
			}
		}
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

	// 目标行（主线方向 + 进度；亲历过都市传说后追加图鉴收集进度）
	if (UMilestoneSubsystem* Milestones = GI ? GI->GetSubsystem<UMilestoneSubsystem>() : nullptr)
	{
		FString Objective = Milestones->GetActiveObjectiveText().ToString();
		if (UHorrorCodexSubsystem* Codex = GI->GetSubsystem<UHorrorCodexSubsystem>())
		{
			if (Codex->GetDiscoveredCount() > 0)
			{
				Objective += FString::Printf(TEXT("   ·   🕯 %s"), *Codex->GetProgressText().ToString());
			}
		}
		HudWidget->SetObjectiveText(FText::FromString(Objective));
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

	// 攀爬态：沿墙面移动。Axis.Y → 沿墙向上/下（世界 Z），Axis.X → 沿墙横向。
	if (bClimbing)
	{
		const FVector Up = FVector::UpVector;
		// 墙面横向 = 墙法线 × 世界上方
		const FVector WallRight = FVector::CrossProduct(ClimbWallNormal, Up).GetSafeNormal();
		AddMovementInput(Up, Axis.Y);
		AddMovementInput(WallRight, Axis.X);
		return;
	}

	// 第一人称：相对控制器（视线）水平朝向移动。Axis.Y 前后，Axis.X 左右。
	const FRotator YawRotation(0.f, Ctrl->GetControlRotation().Yaw, 0.f);
	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(Forward, Axis.Y);
	AddMovementInput(Right, Axis.X);
}

void ASGPlayerCharacter::ToggleClimb()
{
	if (bClimbing) { StopClimb(); }
	else           { StartClimb(); }
}

void ASGPlayerCharacter::StartClimb()
{
	// 朝视线前方射线，够近且打到接近竖直的面（墙）才允许攀爬。
	const AController* Ctrl = GetController();
	if (!Ctrl) { return; }

	const FVector Start = GetActorLocation();
	const FVector Forward = Ctrl->GetControlRotation().Vector();
	const FVector End = Start + Forward * 100.f; // 1m 内有墙才贴

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		return; // 前方无墙
	}
	// 墙面近竖直：法线水平分量大（|normal.Z| 小）
	if (FMath::Abs(Hit.Normal.Z) > 0.5f)
	{
		return; // 是地面/斜坡，不是墙
	}

	ClimbWallNormal = Hit.Normal;
	bClimbing = true;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->SetMovementMode(MOVE_Flying); // 关重力，能沿墙上下
		Move->StopMovementImmediately();
	}
}

void ASGPlayerCharacter::StopClimb()
{
	bClimbing = false;
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->SetMovementMode(MOVE_Falling); // 松手→下落，落地自动转 Walking
	}
}

void ASGPlayerCharacter::TickClimb()
{
	// 每帧确认还贴着墙：朝墙法线反方向探，丢了墙（爬到顶/侧边离开）就退出攀爬。
	const FVector Start = GetActorLocation();
	const FVector End = Start - ClimbWallNormal * 120.f; // 朝墙探 1.2m
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	const bool bStillWall = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params)
		&& FMath::Abs(Hit.Normal.Z) <= 0.5f;

	if (!bStillWall)
	{
		// 爬过墙顶：给个向前的小推力翻越上去，再退出攀爬。
		if (UCharacterMovementComponent* Move = GetCharacterMovement())
		{
			Move->Launch(-ClimbWallNormal * 200.f + FVector(0, 0, 300.f)); // 朝墙内+上方翻越
		}
		StopClimb();
		return;
	}

	// 贴墙：把自己吸附到离墙固定距离，避免飘离。
	const FVector Desired = Hit.ImpactPoint + ClimbWallNormal * 45.f; // 距墙 45cm
	SetActorLocation(FVector(Desired.X, Desired.Y, Start.Z), /*bSweep=*/false);
}

void ASGPlayerCharacter::Punch()
{
	// 朝视线前方 2m 射线，打到 StreetNPC 就造成伤害（占位近战，无动画）。
	const AController* Ctrl = GetController();
	if (!Ctrl) { return; }

	const FVector Start = GetActorLocation();
	const FVector Forward = Ctrl->GetControlRotation().Vector();
	const FVector End = Start + Forward * 200.f;

	// 出拳音（E 块音效）。
	if (UGameInstance* GI = GetGameInstance())
	{
		if (USGAudioSubsystem* Audio = GI->GetSubsystem<USGAudioSubsystem>())
		{
			Audio->PlayCue2D(ESGSound::Punch);
		}
	}

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Pawn, Params))
	{
		if (ASGStreetNPC* Npc = Cast<ASGStreetNPC>(Hit.GetActor()))
		{
			Npc->TakeMeleeHit(34); // 三拳打倒一个路人（100 血）
			if (UGameInstance* GI = GetGameInstance())
			{
				if (USGAudioSubsystem* Audio = GI->GetSubsystem<USGAudioSubsystem>())
				{
					Audio->PlayCueAtLocation(ESGSound::NpcHit, Hit.ImpactPoint);
				}
			}
		}
	}

	// 出拳反馈：HUD 气泡（占位，无打击动画）。
	if (HudWidget)
	{
		HudWidget->SetDialogueText(FText::FromString(TEXT("👊 出拳")));
		FTimerDelegate ClearDel = FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (HudWidget) { HudWidget->SetDialogueText(FText::GetEmpty()); }
		});
		GetWorldTimerManager().SetTimer(DialogueClearTimer, ClearDel, 1.f, /*bLoop=*/false);
	}
}

void ASGPlayerCharacter::Fire()
{
	UGameInstance* GI = GetGameInstance();
	UWeaponSubsystem* Weapon = GI ? GI->GetSubsystem<UWeaponSubsystem>() : nullptr;
	if (!Weapon) { return; }

	// 没枪：提示去搞一把（徒手用 F 出拳）。
	if (!Weapon->HasGun())
	{
		if (HudWidget)
		{
			HudWidget->SetDialogueText(FText::FromString(TEXT("✋ 徒手——按 F 出拳，或先搞把枪")));
			FTimerDelegate D = FTimerDelegate::CreateWeakLambda(this, [this]()
			{ if (HudWidget) { HudWidget->SetDialogueText(FText::GetEmpty()); } });
			GetWorldTimerManager().SetTimer(DialogueClearTimer, D, 1.5f, false);
		}
		return;
	}

	int32 Damage = 0, WantedPerShot = 0; float Range = 0.f;
	if (!Weapon->TryFire(Damage, Range, WantedPerShot))
	{
		// 弹匣空：提示换弹。
		if (HudWidget)
		{
			HudWidget->SetDialogueText(FText::FromString(TEXT("🔫 没子弹了——按 R 换弹")));
			FTimerDelegate D = FTimerDelegate::CreateWeakLambda(this, [this]()
			{ if (HudWidget) { HudWidget->SetDialogueText(FText::GetEmpty()); } });
			GetWorldTimerManager().SetTimer(DialogueClearTimer, D, 1.5f, false);
		}
		return;
	}

	// 枪声（E 块音效）。
	if (USGAudioSubsystem* Audio = GI->GetSubsystem<USGAudioSubsystem>())
	{
		Audio->PlayCue2D(ESGSound::Gunshot);
	}

	// 从相机（眼睛）沿视线射线，命中 StreetNPC 造成伤害。
	const FVector Start = FirstPersonCamera ? FirstPersonCamera->GetComponentLocation() : GetActorLocation();
	const FVector Dir = GetController() ? GetController()->GetControlRotation().Vector() : GetActorForwardVector();
	const FVector End = Start + Dir * Range;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Pawn, Params))
	{
		if (ASGStreetNPC* Npc = Cast<ASGStreetNPC>(Hit.GetActor()))
		{
			Npc->TakeMeleeHit(Damage); // 复用通用受击入口（掉血 + 内部涨通缉）
			if (USGAudioSubsystem* Audio = GI->GetSubsystem<USGAudioSubsystem>())
			{
				Audio->PlayCueAtLocation(ESGSound::NpcHit, Hit.ImpactPoint);
			}
		}
	}

	// 开枪本身就是大事：额外涨通缉（哪怕没打中，街上听到枪声）。
	if (UWantedSubsystem* Wanted = GI->GetSubsystem<UWantedSubsystem>())
	{
		Wanted->AddHeat(WantedPerShot);
	}
}

void ASGPlayerCharacter::ReloadWeapon()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UWeaponSubsystem* Weapon = GI->GetSubsystem<UWeaponSubsystem>())
		{
			if (Weapon->HasGun())
			{
				Weapon->Reload();
				if (USGAudioSubsystem* Audio = GI->GetSubsystem<USGAudioSubsystem>())
				{
					Audio->PlayCue2D(ESGSound::Reload);
				}
				if (HudWidget)
				{
					HudWidget->SetDialogueText(FText::FromString(TEXT("🔁 换弹")));
					FTimerDelegate D = FTimerDelegate::CreateWeakLambda(this, [this]()
					{ if (HudWidget) { HudWidget->SetDialogueText(FText::GetEmpty()); } });
					GetWorldTimerManager().SetTimer(DialogueClearTimer, D, 1.f, false);
				}
			}
		}
	}
}

void ASGPlayerCharacter::StartSprint()
{
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = SprintWalkSpeed;
	}
}

void ASGPlayerCharacter::StopSprint()
{
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = NormalWalkSpeed;
	}
}

void ASGPlayerCharacter::ToggleCrouch()
{
	// 攀爬态不蹲。
	if (bClimbing) { return; }
	if (bIsCrouched) { UnCrouch(); }
	else { Crouch(); }
}

void ASGPlayerCharacter::ToggleView()
{
	bThirdPerson = !bThirdPerson;
	if (FirstPersonCamera)
	{
		FirstPersonCamera->SetRelativeLocation(
			bThirdPerson ? ThirdPersonCameraOffset : FirstPersonCameraOffset);
	}
}
