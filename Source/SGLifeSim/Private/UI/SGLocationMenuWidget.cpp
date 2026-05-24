#include "UI/SGLocationMenuWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "Systems/AssetsSubsystem.h"
#include "Systems/AssetsTypes.h"
#include "Systems/CareerSubsystem.h"
#include "Systems/SaveGameSubsystem.h"
#include "Systems/NightCommuteSubsystem.h"
#include "Systems/NightCommuteTypes.h"
#include "UI/SGActivityMenuWidget.h"
#include "World/LocationManagerSubsystem.h"
#include "World/LocationRegistry.h"
#include "World/SGDrivableCar.h"
#include "Systems/EconomySubsystem.h"
#include "Systems/EconomyTypes.h"
#include "Systems/PlayerStateSubsystem.h"
#include "Systems/PlayerStatsTypes.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	// 造一个带居中文字的按钮
	UButton* MakeButton(UWidgetTree* Tree, const FString& Label, const FName& Name)
	{
		UButton* Btn = Tree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
		UTextBlock* Txt = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Txt->SetText(FText::FromString(Label));
		Txt->SetJustification(ETextJustify::Center);
		FSlateFontInfo Font = Txt->GetFont();
		Font.Size = 22;
		Txt->SetFont(Font);
		Btn->AddChild(Txt);
		return Btn;
	}
}

TSharedRef<SWidget> USGLocationMenuWidget::RebuildWidget()
{
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}

	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(GetRootWidget());
	if (!RootCanvas && WidgetTree)
	{
		RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
		WidgetTree->RootWidget = RootCanvas;

		// 半透明黑底，居中
		UBorder* Backdrop = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Backdrop"));
		Backdrop->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.65f));
		Backdrop->SetPadding(FMargin(48.f));
		if (UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(Backdrop))
		{
			CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f));
			CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			CanvasSlot->SetAutoSize(true);
			CanvasSlot->SetPosition(FVector2D(0.f, 0.f));
		}

		UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("VBox"));
		Backdrop->AddChild(VBox);

		UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Title"));
		Title->SetText(FText::FromString(TEXT("菜单")));
		Title->SetJustification(ETextJustify::Center);
		{
			FSlateFontInfo Font = Title->GetFont();
			Font.Size = 28;
			Title->SetFont(Font);
		}
		if (UVerticalBoxSlot* BoxSlot = VBox->AddChildToVerticalBox(Title))
		{
			BoxSlot->SetHorizontalAlignment(HAlign_Center);
			BoxSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 18.f));
		}

		// 夜归抉择入口（鬼月深夜才可见）：放最顶，营造「先要过这一关」的压迫感。
		NightCommuteButton = MakeButton(WidgetTree, TEXT("🛗 这电梯……停在 13 楼，门开着"), TEXT("NightCommuteButton"));
		if (UVerticalBoxSlot* BoxSlot = VBox->AddChildToVerticalBox(NightCommuteButton))
		{
			BoxSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 14.f));
			BoxSlot->SetHorizontalAlignment(HAlign_Fill);
		}
		NightCommuteButton->OnClicked.AddDynamic(this, &USGLocationMenuWidget::OnNightCommuteClicked);

		CommuteWaitButton = MakeButton(WidgetTree, TEXT("等下一趟（别进去）"), TEXT("CommuteWaitButton"));
		if (UVerticalBoxSlot* BoxSlot = VBox->AddChildToVerticalBox(CommuteWaitButton))
		{
			BoxSlot->SetPadding(FMargin(0.f, 4.f));
			BoxSlot->SetHorizontalAlignment(HAlign_Fill);
		}
		CommuteWaitButton->OnClicked.AddDynamic(this, &USGLocationMenuWidget::OnCommuteWaitClicked);

		CommuteStepInButton = MakeButton(WidgetTree, TEXT("管它呢，赶紧进去"), TEXT("CommuteStepInButton"));
		if (UVerticalBoxSlot* BoxSlot = VBox->AddChildToVerticalBox(CommuteStepInButton))
		{
			BoxSlot->SetPadding(FMargin(0.f, 4.f));
			BoxSlot->SetHorizontalAlignment(HAlign_Fill);
		}
		CommuteStepInButton->OnClicked.AddDynamic(this, &USGLocationMenuWidget::OnCommuteStepInClicked);

		CommuteStairsButton = MakeButton(WidgetTree, TEXT("走楼梯上去（很累）"), TEXT("CommuteStairsButton"));
		if (UVerticalBoxSlot* BoxSlot = VBox->AddChildToVerticalBox(CommuteStairsButton))
		{
			BoxSlot->SetPadding(FMargin(0.f, 4.f, 0.f, 14.f));
			BoxSlot->SetHorizontalAlignment(HAlign_Fill);
		}
		CommuteStairsButton->OnClicked.AddDynamic(this, &USGLocationMenuWidget::OnCommuteStairsClicked);

		// 出门回城市（仅室内关卡显示）。城市里靠走门口按 E 进楼，不在菜单里瞬移。
			ExitToCityButton = MakeButton(WidgetTree, TEXT("出门（回大街）"), TEXT("ExitToCityButton"));
			if (UVerticalBoxSlot* BoxSlot = VBox->AddChildToVerticalBox(ExitToCityButton))
			{
				BoxSlot->SetPadding(FMargin(0.f, 4.f));
				BoxSlot->SetHorizontalAlignment(HAlign_Fill);
			}
			ExitToCityButton->OnClicked.AddDynamic(this, &USGLocationMenuWidget::OnExitToCity);

		SaveButton = MakeButton(WidgetTree, TEXT("存档"), TEXT("SaveButton"));
		if (UVerticalBoxSlot* BoxSlot = VBox->AddChildToVerticalBox(SaveButton))
		{
			BoxSlot->SetPadding(FMargin(0.f, 14.f, 0.f, 4.f));
			BoxSlot->SetHorizontalAlignment(HAlign_Fill);
		}
		SaveButton->OnClicked.AddDynamic(this, &USGLocationMenuWidget::OnSaveClicked);

		LoadButton = MakeButton(WidgetTree, TEXT("读档"), TEXT("LoadButton"));
		if (UVerticalBoxSlot* BoxSlot = VBox->AddChildToVerticalBox(LoadButton))
		{
			BoxSlot->SetPadding(FMargin(0.f, 4.f));
			BoxSlot->SetHorizontalAlignment(HAlign_Fill);
		}
		LoadButton->OnClicked.AddDynamic(this, &USGLocationMenuWidget::OnLoadClicked);

		BuyHdbButton = MakeButton(WidgetTree, TEXT("按揭买组屋（首付 25%）"), TEXT("BuyHdbButton"));
		if (UVerticalBoxSlot* BoxSlot = VBox->AddChildToVerticalBox(BuyHdbButton))
		{
			BoxSlot->SetPadding(FMargin(0.f, 14.f, 0.f, 4.f));
			BoxSlot->SetHorizontalAlignment(HAlign_Fill);
		}
		BuyHdbButton->OnClicked.AddDynamic(this, &USGLocationMenuWidget::OnBuyHdbFinancedClicked);

		PrepayButton = MakeButton(WidgetTree, TEXT("提前还清房贷"), TEXT("PrepayButton"));
		if (UVerticalBoxSlot* BoxSlot = VBox->AddChildToVerticalBox(PrepayButton))
		{
			BoxSlot->SetPadding(FMargin(0.f, 4.f));
			BoxSlot->SetHorizontalAlignment(HAlign_Fill);
		}
		PrepayButton->OnClicked.AddDynamic(this, &USGLocationMenuWidget::OnPrepayMortgageClicked);

		PromoteButton = MakeButton(WidgetTree, TEXT("申请升职"), TEXT("PromoteButton"));
		if (UVerticalBoxSlot* BoxSlot = VBox->AddChildToVerticalBox(PromoteButton))
		{
			BoxSlot->SetPadding(FMargin(0.f, 14.f, 0.f, 4.f));
			BoxSlot->SetHorizontalAlignment(HAlign_Fill);
		}
		PromoteButton->OnClicked.AddDynamic(this, &USGLocationMenuWidget::OnPromoteClicked);

		JobHopButton = MakeButton(WidgetTree, TEXT("跳槽（+35%）"), TEXT("JobHopButton"));
		if (UVerticalBoxSlot* BoxSlot = VBox->AddChildToVerticalBox(JobHopButton))
		{
			BoxSlot->SetPadding(FMargin(0.f, 4.f));
			BoxSlot->SetHorizontalAlignment(HAlign_Fill);
		}
		JobHopButton->OnClicked.AddDynamic(this, &USGLocationMenuWidget::OnJobHopClicked);

		ActivitiesButton = MakeButton(WidgetTree, TEXT("做点事…（在这儿过日子）"), TEXT("ActivitiesButton"));
		if (UVerticalBoxSlot* BoxSlot = VBox->AddChildToVerticalBox(ActivitiesButton))
		{
			BoxSlot->SetPadding(FMargin(0.f, 14.f, 0.f, 4.f));
			BoxSlot->SetHorizontalAlignment(HAlign_Fill);
		}
		ActivitiesButton->OnClicked.AddDynamic(this, &USGLocationMenuWidget::OnDoActivitiesClicked);

		// 消费（第8块）：买车 / 下馆子，给钱更多出口。
		BuyCarButton = MakeButton(WidgetTree, TEXT("买辆车（$50k）"), TEXT("BuyCarButton"));
		if (UVerticalBoxSlot* BoxSlot = VBox->AddChildToVerticalBox(BuyCarButton))
		{
			BoxSlot->SetPadding(FMargin(0.f, 14.f, 0.f, 4.f));
			BoxSlot->SetHorizontalAlignment(HAlign_Fill);
		}
		BuyCarButton->OnClicked.AddDynamic(this, &USGLocationMenuWidget::OnBuyCarClicked);

		DineOutButton = MakeButton(WidgetTree, TEXT("下馆子犒劳自己（$80，回心情）"), TEXT("DineOutButton"));
		if (UVerticalBoxSlot* BoxSlot = VBox->AddChildToVerticalBox(DineOutButton))
		{
			BoxSlot->SetPadding(FMargin(0.f, 4.f));
			BoxSlot->SetHorizontalAlignment(HAlign_Fill);
		}
		DineOutButton->OnClicked.AddDynamic(this, &USGLocationMenuWidget::OnDineOutClicked);

		CloseButton = MakeButton(WidgetTree, TEXT("取消  ·  M"), TEXT("CloseButton"));
		if (UVerticalBoxSlot* BoxSlot = VBox->AddChildToVerticalBox(CloseButton))
		{
			BoxSlot->SetPadding(FMargin(0.f, 14.f, 0.f, 0.f));
			BoxSlot->SetHorizontalAlignment(HAlign_Fill);
		}
		CloseButton->OnClicked.AddDynamic(this, &USGLocationMenuWidget::OnCloseClicked);

		// 操作反馈行（存/读档结果），初始空。
		StatusLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StatusLabel"));
		StatusLabel->SetText(FText::GetEmpty());
		StatusLabel->SetJustification(ETextJustify::Center);
		StatusLabel->SetColorAndOpacity(FSlateColor(FLinearColor(0.6f, 1.f, 0.7f)));
		if (UVerticalBoxSlot* BoxSlot = VBox->AddChildToVerticalBox(StatusLabel))
		{
			BoxSlot->SetPadding(FMargin(0.f, 12.f, 0.f, 0.f));
			BoxSlot->SetHorizontalAlignment(HAlign_Center);
		}
	}

	return Super::RebuildWidget();
}

void USGLocationMenuWidget::SetStatus(const FString& Message)
{
	if (StatusLabel)
	{
		StatusLabel->SetText(FText::FromString(Message));
	}
}

void USGLocationMenuWidget::OpenMenu()
{
	if (!IsInViewport())
	{
		AddToViewport(100);
	}
	// 每次打开都从普通态起步，按当前是否鬼月深夜决定露不露夜归入口。
	bInNightCommute = false;
	RefreshButtonVisibility();
	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeGameAndUI Mode;
		Mode.SetWidgetToFocus(TakeWidget());
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(Mode);
		PC->SetShowMouseCursor(true);
	}
}

void USGLocationMenuWidget::CloseMenu()
{
	TravelTo(NAME_None);
}

void USGLocationMenuWidget::TravelTo(FName LevelName)
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
	}
	RemoveFromParent();

	if (!LevelName.IsNone())
	{
		UGameplayStatics::OpenLevel(this, LevelName);
	}
}

void USGLocationMenuWidget::OnExitToCity()
{
	// 恢复游戏输入、关菜单，再让 LocationManager 把我们送回城市枢纽
	// （ReturnToCity 内部 OpenLevel L_City，并标记回程把玩家传送回离开时的那栋楼门口）。
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
	}
	RemoveFromParent();

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (UGameInstance* GI = PC->GetGameInstance())
		{
			if (USGLocationManagerSubsystem* Loc = GI->GetSubsystem<USGLocationManagerSubsystem>())
			{
				Loc->ReturnToCity();
			}
		}
	}
}

void USGLocationMenuWidget::OnSaveClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (UGameInstance* GI = PC->GetGameInstance())
		{
			if (USaveGameSubsystem* SaveSys = GI->GetSubsystem<USaveGameSubsystem>())
			{
				const bool bOk = SaveSys->SaveToSlot(USaveGameSubsystem::DefaultSlot);
				SetStatus(bOk ? TEXT("已存档 ✓") : TEXT("存档失败"));
				return;
			}
		}
	}
	SetStatus(TEXT("存档失败"));
}

void USGLocationMenuWidget::OnLoadClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (UGameInstance* GI = PC->GetGameInstance())
		{
			if (USaveGameSubsystem* SaveSys = GI->GetSubsystem<USaveGameSubsystem>())
			{
				const bool bOk = SaveSys->LoadFromSlot(USaveGameSubsystem::DefaultSlot);
				// 读档后各 Subsystem 状态已回灌，关菜单回到游戏 → HUD 下一帧反映恢复值。
				SetStatus(bOk ? TEXT("已读档 ✓") : TEXT("没有存档"));
				return;
			}
		}
	}
	SetStatus(TEXT("读档失败"));
}

void USGLocationMenuWidget::OnBuyHdbFinancedClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (UGameInstance* GI = PC->GetGameInstance())
		{
			if (UAssetsSubsystem* Assets = GI->GetSubsystem<UAssetsSubsystem>())
			{
				if (Assets->GetHousingTier() == EHousingTier::OwnedHDB || Assets->HasMortgage())
				{
					SetStatus(TEXT("已有组屋 / 房贷"));
					return;
				}
				const bool bOk = Assets->BuyHousingFinanced(EHousingTier::OwnedHDB);
				SetStatus(bOk ? TEXT("已按揭购入组屋 ✓") : TEXT("首付不够，买不起"));
				return;
			}
		}
	}
	SetStatus(TEXT("购房失败"));
}

void USGLocationMenuWidget::OnPrepayMortgageClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (UGameInstance* GI = PC->GetGameInstance())
		{
			if (UAssetsSubsystem* Assets = GI->GetSubsystem<UAssetsSubsystem>())
			{
				if (!Assets->HasMortgage())
				{
					SetStatus(TEXT("当前无房贷"));
					return;
				}
				const bool bOk = Assets->PrepayMortgage();
				SetStatus(bOk ? TEXT("已结清房贷 ✓") : TEXT("现金不够结清"));
				return;
			}
		}
	}
	SetStatus(TEXT("操作失败"));
}

void USGLocationMenuWidget::OnPromoteClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (UGameInstance* GI = PC->GetGameInstance())
		{
			if (UCareerSubsystem* Career = GI->GetSubsystem<UCareerSubsystem>())
			{
				const bool bOk = Career->TryPromote();
				if (bOk)
				{
					SetStatus(FString::Printf(TEXT("升职成功 → %s ✓"),
						*UEnum::GetDisplayValueAsText(Career->GetLevel()).ToString()));
				}
				else
				{
					SetStatus(TEXT("暂不够格（需更高专业技能 / 在职满 3 月）"));
				}
				return;
			}
		}
	}
	SetStatus(TEXT("操作失败"));
}

void USGLocationMenuWidget::OnJobHopClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (UGameInstance* GI = PC->GetGameInstance())
		{
			if (UCareerSubsystem* Career = GI->GetSubsystem<UCareerSubsystem>())
			{
				const bool bOk = Career->JobHop();
				SetStatus(bOk ? TEXT("跳槽成功，薪资 +35% ✓") : TEXT("待业中，无处可跳"));
				return;
			}
		}
	}
	SetStatus(TEXT("操作失败"));
}

void USGLocationMenuWidget::OnDoActivitiesClicked()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}
	if (!ActivityMenu)
	{
		ActivityMenu = CreateWidget<USGActivityMenuWidget>(PC, USGActivityMenuWidget::StaticClass());
	}
	if (ActivityMenu)
	{
		// 关掉地点菜单（不旅行），打开活动菜单接管输入。
		RemoveFromParent();
		ActivityMenu->OpenMenu();
	}
}

void USGLocationMenuWidget::OnBuyCarClicked()
{
	APlayerController* PC = GetOwningPlayer();
	UGameInstance* GI = PC ? PC->GetGameInstance() : nullptr;
	UEconomySubsystem* Eco = GI ? GI->GetSubsystem<UEconomySubsystem>() : nullptr;
	if (!Eco) { SetStatus(TEXT("买车失败")); return; }

	// 50k = 5,000,000 分
	if (!Eco->TryWithdraw(ECurrencyAccount::Cash, 5000000, TEXT("BuyCar")))
	{
		SetStatus(TEXT("现金不够买车（$50k）"));
		return;
	}

	// 在玩家身旁生成一辆可驾驶车。
	if (UWorld* World = GI->GetWorld())
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			const FVector Spawn = Pawn->GetActorLocation() + Pawn->GetActorForwardVector() * 350.f + FVector(0,0,50);
			FActorSpawnParameters P; P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			World->SpawnActor<ASGDrivableCar>(ASGDrivableCar::StaticClass(), Spawn, FRotator::ZeroRotator, P);
		}
	}
	SetStatus(TEXT("买到车了！走近按 E 上车 ✓"));
}

void USGLocationMenuWidget::OnDineOutClicked()
{
	APlayerController* PC = GetOwningPlayer();
	UGameInstance* GI = PC ? PC->GetGameInstance() : nullptr;
	UEconomySubsystem* Eco = GI ? GI->GetSubsystem<UEconomySubsystem>() : nullptr;
	if (!Eco) { SetStatus(TEXT("操作失败")); return; }

	if (!Eco->TryWithdraw(ECurrencyAccount::Cash, 8000, TEXT("DineOut"))) // $80
	{
		SetStatus(TEXT("现金不够下馆子（$80）"));
		return;
	}
	if (UPlayerStateSubsystem* PS = GI->GetSubsystem<UPlayerStateSubsystem>())
	{
		PS->ModifyAttribute(EPlayerAttribute::Mood, +15);
	}
	SetStatus(TEXT("吃了顿好的，心情 +15 ✓"));
}

void USGLocationMenuWidget::OnCloseClicked()
{
	CloseMenu();
}

bool USGLocationMenuWidget::IsNightCommuteAvailable() const
{
	if (const APlayerController* PC = GetOwningPlayer())
	{
		if (const UGameInstance* GI = PC->GetGameInstance())
		{
			if (const UNightCommuteSubsystem* NC = GI->GetSubsystem<UNightCommuteSubsystem>())
			{
				return NC->IsAvailable();
			}
		}
	}
	return false;
}

void USGLocationMenuWidget::RefreshButtonVisibility()
{
	// 抉择态：只露三个选项 + 状态行，藏起一切日常项（让玩家无从逃避这一刻）。
	// 普通态：露日常项；夜归入口仅在鬼月深夜出现。
	const ESlateVisibility DailyVis    = bInNightCommute ? ESlateVisibility::Collapsed : ESlateVisibility::Visible;
	const ESlateVisibility ChoiceVis   = bInNightCommute ? ESlateVisibility::Visible   : ESlateVisibility::Collapsed;
	const ESlateVisibility EntranceVis = (!bInNightCommute && IsNightCommuteAvailable())
		? ESlateVisibility::Visible : ESlateVisibility::Collapsed;

	// 「出门」只在室内关卡有意义；人已在城市枢纽时藏起来（城市靠走门口按 E 进楼）。
	const FString CurLevel = UGameplayStatics::GetCurrentLevelName(this, /*bRemovePrefix=*/true);
	const bool bInCity = CurLevel.Contains(FLocationRegistry::GetCityLevelName().ToString());
	const ESlateVisibility ExitVis = (!bInNightCommute && !bInCity)
		? ESlateVisibility::Visible : ESlateVisibility::Collapsed;

	auto SetVis = [](UButton* Btn, ESlateVisibility Vis) { if (Btn) { Btn->SetVisibility(Vis); } };

	SetVis(NightCommuteButton, EntranceVis);

	SetVis(CommuteWaitButton, ChoiceVis);
	SetVis(CommuteStepInButton, ChoiceVis);
	SetVis(CommuteStairsButton, ChoiceVis);

	SetVis(ExitToCityButton, ExitVis);
	SetVis(SaveButton, DailyVis);
	SetVis(LoadButton, DailyVis);
	SetVis(BuyHdbButton, DailyVis);
	SetVis(PrepayButton, DailyVis);
	SetVis(PromoteButton, DailyVis);
	SetVis(JobHopButton, DailyVis);
	SetVis(ActivitiesButton, DailyVis);
	SetVis(BuyCarButton, DailyVis);
	SetVis(DineOutButton, DailyVis);
}

void USGLocationMenuWidget::OnNightCommuteClicked()
{
	bInNightCommute = true;
	RefreshButtonVisibility();
	SetStatus(TEXT("Uncle Lim 说过：别进去……"));
}

void USGLocationMenuWidget::ResolveNightCommute(uint8 Choice)
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (UGameInstance* GI = PC->GetGameInstance())
		{
			if (UNightCommuteSubsystem* NC = GI->GetSubsystem<UNightCommuteSubsystem>())
			{
				NC->MakeChoice((ENightCommuteChoice)Choice);
			}
		}
	}
	// 结算文案由 PlayerCharacter 订阅 OnResolved 弹气泡；关菜单回游戏即可。
	CloseMenu();
}

void USGLocationMenuWidget::OnCommuteWaitClicked()
{
	ResolveNightCommute((uint8)ENightCommuteChoice::WaitForNext);
}

void USGLocationMenuWidget::OnCommuteStepInClicked()
{
	ResolveNightCommute((uint8)ENightCommuteChoice::StepIn);
}

void USGLocationMenuWidget::OnCommuteStairsClicked()
{
	ResolveNightCommute((uint8)ENightCommuteChoice::TakeStairs);
}
