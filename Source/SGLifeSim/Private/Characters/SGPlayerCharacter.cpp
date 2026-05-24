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
#include "Systems/TimeBlock.h"
#include "Systems/TimeSubsystem.h"

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

	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
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
	if (Nearest && Nearest->Implements<UInteractableInterface>())
	{
		IInteractableInterface::Execute_OnInteract(Nearest, this);
	}
	else if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Silver, TEXT("附近没有可交互的人"));
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
	// 原型：M 在出租屋 / 食阁两个关卡间直接切换（替代 UMG 跳转菜单）。
	// TimeSubsystem 在 GameInstance 上，跨关卡保留 —— 验证 ADR 0005。
	const FString Current = GetWorld() ? GetWorld()->GetMapName() : FString();
	const bool bAtHawker = Current.Contains(TEXT("HawkerCenter"));
	const FName Target = bAtHawker ? FName(TEXT("L_Rental")) : FName(TEXT("L_HawkerCenter"));
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan,
			FString::Printf(TEXT("前往：%s"), *Target.ToString()));
	}
	UGameplayStatics::OpenLevel(this, Target);
}

void ASGPlayerCharacter::DrawPrototypeHUD()
{
	if (!GEngine)
	{
		return;
	}
	UGameInstance* GI = GetGameInstance();
	UTimeSubsystem* TimeSys = GI ? GI->GetSubsystem<UTimeSubsystem>() : nullptr;
	if (!TimeSys)
	{
		return;
	}

	const FText BlockText = UEnum::GetDisplayValueAsText(TimeSys->GetCurrentBlock());
	const FText WeekdayText = UEnum::GetDisplayValueAsText(TimeSys->GetWeekday());
	const FString Hud = FString::Printf(TEXT("Day %d · %s · %s    [E] 交谈  [T] 推进时间  [M] 切换地点"),
		TimeSys->GetDayNumber(), *WeekdayText.ToString(), *BlockText.ToString());

	// 固定 Key=1 让这行原地刷新而不是堆叠
	GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, Hud);

	// 靠近可交互对象时显示其提示（如「[E] 对话」），Key=3 原地刷新
	AActor* Nearby = FindNearbyInteractable();
	if (Nearby && Nearby->Implements<UInteractableInterface>())
	{
		const FText Prompt = IInteractableInterface::Execute_GetInteractionPrompt(Nearby);
		GEngine->AddOnScreenDebugMessage(3, 0.f, FColor::Green, Prompt.ToString());
	}
}

void ASGPlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	// 等距视角下，2D 输入直接映射到世界 X/Y 平面：X 左右，Y 前后
	AddMovementInput(FVector(1.f, 0.f, 0.f), Axis.X);
	AddMovementInput(FVector(0.f, 1.f, 0.f), Axis.Y);
}
