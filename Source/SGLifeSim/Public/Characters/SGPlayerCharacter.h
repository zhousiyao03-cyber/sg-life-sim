#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SGPlayerCharacter.generated.h"

class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UAnimSequence;
class USGHudWidget;
class USGLocationMenuWidget;
class USGDialogueWidget;
class USGEndingWidget;
class USGMinimapWidget;
enum class EEnding : uint8;
struct FInputActionValue;

/**
 * 主角基类。spec §10.3「C++ 核心 + Blueprint 薄壳」。
 *
 * 第一人称视角（贯穿始终）：相机挂在胶囊眼高、随控制器朝向转；鼠标转视角、
 * WASD 相对视线方向移动。Blueprint 子类（BP_PlayerCharacter）只负责设 SkeletalMesh /
 * AnimClass，并把 IMC/IA 资产引用赋给下面暴露的属性。
 *
 * 选第一人称：恐怖方向的固定视角 —— 受限视野（看不清、身后未知）天然出恐怖，
 * 也让弱美术藏在黑暗 / 手电光锥 / 雾里。等距正交相机看得到全场，反恐怖，已弃用。
 */
UCLASS()
class SGLIFESIM_API ASGPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASGPlayerCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** 第一人称相机：挂在胶囊眼高，bUsePawnControlRotation 随控制器朝向（鼠标）转。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Camera")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	/** 在 BP 子类里赋值为 IMC_Default。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** 在 BP 子类里赋值为 IA_Move（Axis2D）。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Input")
	TObjectPtr<UInputAction> MoveAction;

	/**
	 * 单节点 locomotion（spec §10.3）：不挂 AnimBlueprint，直接在 C++ 里按速度
	 * 切换播放下面两个序列。BP 子类把它们赋为 A_Idle / A_Walk。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Animation")
	TObjectPtr<UAnimSequence> IdleAnim;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Animation")
	TObjectPtr<UAnimSequence> WalkAnim;

	/** 速度高于此阈值视为「在走」，低于则「站立」。cm/s。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SGLifeSim|Animation")
	float WalkSpeedThreshold = 10.f;

private:
	/** IA_Move 的 Triggered 回调：把 2D 输入映射到世界 X/Y 平面移动。攀爬态改为沿墙面移动。 */
	void Move(const FInputActionValue& Value);

	/** 攀爬键（C）：贴墙时切入/退出攀爬模式。 */
	void ToggleClimb();

	/** 出拳键（F）：朝视线前方近战，打到 StreetNPC 造成伤害（第9块）。 */
	void Punch();

	/** 进入攀爬：前方有墙才成立。关重力、贴墙。 */
	void StartClimb();

	/** 退出攀爬：恢复 Walking。 */
	void StopClimb();

	/** 每帧：攀爬态下检测是否还贴着墙、是否爬到顶（翻越/掉落）。 */
	void TickClimb();

	/** 是否正在攀爬。 */
	bool bClimbing = false;

	/** 攀爬贴附的墙面法线（指向玩家），用于贴墙和沿墙移动。 */
	FVector ClimbWallNormal = FVector::ZeroVector;

	/** 按当前水平速度在 Idle / Walk 间切换单节点播放，避免每帧重复 Play。 */
	void UpdateLocomotionAnimation();

	/** 返回交互范围内最近的可交互 Actor（无则 nullptr）。交互与 HUD 提示共用。 */
	AActor* FindNearbyInteractable() const;

	/** IA_Interact（E）：找范围内最近的可交互对象并触发 OnInteract。 */
	void TryInteract();

	/** IA_AdvanceTime（T）：推进一个时间块（经 UTimeSubsystem）。 */
	void AdvanceTime();

	/** IA_OpenLocationMenu（M）：开/关游戏菜单（存读档/活动/出门回城市等）。 */
	void SwitchLocation();

	/** 每帧刷新 HUD widget 的状态行 + 交互提示 + 钱包 / 属性。 */
	void DrawPrototypeHUD();

	/** 绑定到 UProgressSubsystem::OnAchievementUnlocked，弹 HUD toast。 */
	UFUNCTION()
	void HandleAchievementUnlocked(FName AchievementId);

	/** 绑定到 UEconomicEventSubsystem::OnEconomicEvent，弹 HUD toast。 */
	UFUNCTION()
	void HandleEconomicEvent(FText Title);

	/** 绑定到 UMilestoneSubsystem::OnMilestoneCompleted，弹庆祝 toast。 */
	UFUNCTION()
	void HandleMilestoneCompleted(EMilestone Milestone);

	/** 绑定到 UHorrorEventSubsystem::OnHorrorEvent，弹阴森 toast。 */
	UFUNCTION()
	void HandleHorrorEvent(FText Title);

	/** 绑定到 UNightCommuteSubsystem::OnResolved，把夜归抉择结算文案弹成阴森气泡。 */
	UFUNCTION()
	void HandleNightCommuteResolved(const FText& Message);

	/** 绑定到 UEndingSubsystem::OnEndingChosen，盖结局演出 overlay。 */
	UFUNCTION()
	void HandleEndingChosen(EEnding Ending);

	/** 缓存当前正在播的序列，状态没变就不重新 Play。 */
	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> CurrentAnim;

	/** 原型 HUD（纯 C++ UMG）。BeginPlay 里 CreateWidget + AddToViewport。 */
	UPROPERTY(Transient)
	TObjectPtr<USGHudWidget> HudWidget;

	/** 地点切换菜单（纯 C++ UMG），M 键开/关。懒创建。 */
	UPROPERTY(Transient)
	TObjectPtr<USGLocationMenuWidget> LocationMenu;

	/** 对话界面（纯 C++ UMG），E 交互有对话树的 NPC 时弹出。懒创建。 */
	UPROPERTY(Transient)
	TObjectPtr<USGDialogueWidget> DialogueWidget;

	/** 结局演出 overlay（纯 C++ UMG），选定结局时盖满屏。懒创建。 */
	UPROPERTY(Transient)
	TObjectPtr<USGEndingWidget> EndingWidget;

	/** 城市小地图（纯 C++ UMG），仅在城市枢纽关卡显示，每帧更新玩家点。懒创建。 */
	UPROPERTY(Transient)
	TObjectPtr<USGMinimapWidget> MinimapWidget;

	/** 对话气泡 N 秒后自动消失的计时器。 */
	FTimerHandle DialogueClearTimer;
};
