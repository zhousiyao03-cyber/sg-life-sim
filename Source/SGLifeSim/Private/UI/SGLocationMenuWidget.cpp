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
		Title->SetText(FText::FromString(TEXT("去哪里？")));
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

		CloseButton = MakeButton(WidgetTree, TEXT("取消  ·  M"), TEXT("CloseButton"));
		if (UVerticalBoxSlot* BoxSlot = VBox->AddChildToVerticalBox(CloseButton))
		{
			BoxSlot->SetPadding(FMargin(0.f, 14.f, 0.f, 0.f));
			BoxSlot->SetHorizontalAlignment(HAlign_Fill);
		}
		CloseButton->OnClicked.AddDynamic(this, &USGLocationMenuWidget::OnCloseClicked);
	}

	return Super::RebuildWidget();
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

void USGLocationMenuWidget::OnCloseClicked()
{
	CloseMenu();
}
