#include "Characters/ElevatorHorrorDirector.h"
#include "Systems/HorrorSequenceSubsystem.h"
#include "Systems/HorrorSceneTypes.h"

#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

AElevatorHorrorDirector::AElevatorHorrorDirector()
{
	PrimaryActorTick.bCanEverTick = false;

	CarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CarMesh"));
	SetRootComponent(CarMesh);

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(CarMesh);

	GhostMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GhostMesh"));
	GhostMesh->SetupAttachment(CarMesh);

	CeilingLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("CeilingLight"));
	CeilingLight->SetupAttachment(CarMesh);
	CeilingLight->SetRelativeLocation(FVector(0.f, 0.f, 220.f));
	CeilingLight->SetIntensity(BaseLightIntensity);
}

void AElevatorHorrorDirector::BeginPlay()
{
	Super::BeginPlay();

	// 只有 Subsystem 确认正在演电梯场景才接管 —— 否则这关被单独打开时不乱动玩家。
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UHorrorSequenceSubsystem* Seq = GI->GetSubsystem<UHorrorSequenceSubsystem>())
		{
			bActive = (Seq->GetActiveScene() == EHorrorScene::Elevator);
		}
	}

	// 占位外观：用引擎自带基础几何体当电梯壳 / 门 / 女鬼剪影。
	// 资产到位后在 BP 子类 / 编辑器里换成真 mesh，时间线代码无需改动。
	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		if (CarMesh)  { CarMesh->SetStaticMesh(Cube);  CarMesh->SetWorldScale3D(FVector(2.5f, 2.5f, 2.6f)); }
		if (DoorMesh) { DoorMesh->SetStaticMesh(Cube);  DoorMesh->SetRelativeScale3D(FVector(0.1f, 1.8f, 2.2f)); DoorMesh->SetRelativeLocation(FVector(130.f, 0.f, 0.f)); }
		if (GhostMesh)
		{
			GhostMesh->SetStaticMesh(Cube);
			GhostMesh->SetRelativeScale3D(FVector(0.3f, 0.6f, 1.8f)); // 瘦高人形占位
			GhostMesh->SetRelativeLocation(FVector(280.f, 0.f, 0.f)); // 门外
			GhostMesh->SetVisibility(false);                          // 平时不可见
		}
	}

	if (CeilingLight)
	{
		BaseLightIntensity = CeilingLight->Intensity;
	}

	if (bActive)
	{
		ScheduleSequence();
	}
}

void AElevatorHorrorDirector::ScheduleSequence()
{
	const TArray<FElevatorBeat> Beats = FElevatorSequenceBeats::GetBeats();
	BeatTimers.SetNum(Beats.Num());

	for (int32 i = 0; i < Beats.Num(); ++i)
	{
		const EElevatorBeat Beat = Beats[i].Beat;
		const float Delay = Beats[i].TimeSeconds;

		FTimerDelegate Del = FTimerDelegate::CreateWeakLambda(this, [this, Beat]()
		{
			RunBeat(Beat);
		});

		if (Delay <= 0.f)
		{
			RunBeat(Beat); // 0s 节拍立即执行
		}
		else
		{
			GetWorldTimerManager().SetTimer(BeatTimers[i], Del, Delay, /*bLoop=*/false);
		}
	}
}

void AElevatorHorrorDirector::RunBeat(EElevatorBeat Beat)
{
	switch (Beat)
	{
	case EElevatorBeat::Enter:
		LockPlayer(true);
		if (CeilingLight) { CeilingLight->SetIntensity(BaseLightIntensity); }
		if (GhostMesh)    { GhostMesh->SetVisibility(false); }
		break;

	case EElevatorBeat::Ding:
		// TODO(资产): 播「叮」音效 + 楼层数字 UMG 乱跳。
		break;

	case EElevatorBeat::FlickerStart:
		if (CeilingLight) { CeilingLight->SetIntensity(BaseLightIntensity * 0.4f); }
		break;

	case EElevatorBeat::BlackOut:
		if (CeilingLight) { CeilingLight->SetIntensity(0.f); }
		break;

	case EElevatorBeat::LightsBackDoorOpens:
		if (CeilingLight) { CeilingLight->SetIntensity(BaseLightIntensity * 0.7f); }
		if (DoorMesh)     { DoorMesh->SetRelativeLocation(FVector(130.f, 200.f, 0.f)); } // 门滑开（占位：瞬移，后续 Lerp）
		break;

	case EElevatorBeat::DroneFootsteps:
		// TODO(资产): 低频 drone + 稀疏脚步声。
		break;

	case EElevatorBeat::GhostReveal:
		if (CeilingLight) { CeilingLight->SetIntensity(BaseLightIntensity * 3.f); } // 爆闪
		if (GhostMesh)    { GhostMesh->SetVisibility(true); }
		break;

	case EElevatorBeat::GhostGone:
		if (CeilingLight) { CeilingLight->SetIntensity(0.f); }
		if (GhostMesh)    { GhostMesh->SetVisibility(false); }
		break;

	case EElevatorBeat::ScareStinger:
		// TODO(资产): 尖锐音效 + 屏幕骤暗后处理。
		if (CeilingLight) { CeilingLight->SetIntensity(BaseLightIntensity * 0.2f); }
		break;

	case EElevatorBeat::DoorCloseReset:
		if (DoorMesh)     { DoorMesh->SetRelativeLocation(FVector(130.f, 0.f, 0.f)); } // 门复位
		if (CeilingLight) { CeilingLight->SetIntensity(BaseLightIntensity); }
		break;

	case EElevatorBeat::Exit:
		LockPlayer(false);
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UHorrorSequenceSubsystem* Seq = GI->GetSubsystem<UHorrorSequenceSubsystem>())
			{
				Seq->ExitScene(); // 结算 + OpenLevel 回原关卡
			}
		}
		break;
	}
}

void AElevatorHorrorDirector::LockPlayer(bool bLock)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		// 只禁移动，保留鼠标转头（受限视野是恐怖的一部分）。
		PC->SetIgnoreMoveInput(bLock);

		// 把玩家挪到电梯中央朝门，锁住期间站定。
		if (bLock)
		{
			if (APawn* Pawn = PC->GetPawn())
			{
				Pawn->SetActorLocation(GetActorLocation() + FVector(0.f, 0.f, 90.f));
			}
		}
	}
}
