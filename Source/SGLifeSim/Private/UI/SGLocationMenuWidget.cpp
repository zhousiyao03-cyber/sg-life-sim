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
#include "Systems/SaveGameSubsystem.h"

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

		RentalButton = MakeButton(WidgetTree, TEXT("出租屋  ·  L_Rental"), TEXT("RentalButton"));
		if (UVerticalBoxSlot* BoxSlot = VBox->AddChildToVerticalBox(RentalButton))
		{
			BoxSlot->SetPadding(FMargin(0.f, 4.f));
			BoxSlot->SetHorizontalAlignment(HAlign_Fill);
		}
		RentalButton->OnClicked.AddDynamic(this, &USGLocationMenuWidget::OnGoRental);

		HawkerButton = MakeButton(WidgetTree, TEXT("食阁  ·  L_HawkerCenter"), TEXT("HawkerButton"));
		if (UVerticalBoxSlot* BoxSlot = VBox->AddChildToVerticalBox(HawkerButton))
		{
			BoxSlot->SetPadding(FMargin(0.f, 4.f));
			BoxSlot->SetHorizontalAlignment(HAlign_Fill);
		}
		HawkerButton->OnClicked.AddDynamic(this, &USGLocationMenuWidget::OnGoHawker);

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

void USGLocationMenuWidget::OnGoRental()
{
	TravelTo(FName(TEXT("L_Rental")));
}

void USGLocationMenuWidget::OnGoHawker()
{
	TravelTo(FName(TEXT("L_HawkerCenter")));
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

void USGLocationMenuWidget::OnCloseClicked()
{
	CloseMenu();
}
