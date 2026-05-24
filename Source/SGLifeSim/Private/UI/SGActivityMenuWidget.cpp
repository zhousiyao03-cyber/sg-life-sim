#include "UI/SGActivityMenuWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Systems/ActivitySubsystem.h"
#include "Systems/ActivitySystem.h"
#include "Systems/PlayerStateSubsystem.h"
#include "Systems/PlayerStatsTypes.h"
#include "Systems/TimeSubsystem.h"
#include "Systems/TimeBlock.h"

namespace
{
	UButton* MakeActivityButton(UWidgetTree* Tree, const FName& Name, UTextBlock*& OutLabel)
	{
		UButton* Btn = Tree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
		OutLabel = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		OutLabel->SetJustification(ETextJustify::Center);
		FSlateFontInfo Font = OutLabel->GetFont();
		Font.Size = 18;
		OutLabel->SetFont(Font);
		Btn->AddChild(OutLabel);
		return Btn;
	}
}

TSharedRef<SWidget> USGActivityMenuWidget::RebuildWidget()
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

		UBorder* Backdrop = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Backdrop"));
		Backdrop->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.7f));
		Backdrop->SetPadding(FMargin(40.f, 28.f));
		if (UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(Backdrop))
		{
			CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f));
			CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			CanvasSlot->SetAutoSize(true);
		}

		UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("VBox"));
		Backdrop->AddChild(VBox);

		TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TitleText"));
		TitleText->SetText(FText::FromString(TEXT("做点什么？")));
		TitleText->SetJustification(ETextJustify::Center);
		{
			FSlateFontInfo Font = TitleText->GetFont();
			Font.Size = 26;
			TitleText->SetFont(Font);
		}
		if (UVerticalBoxSlot* BoxSlot = VBox->AddChildToVerticalBox(TitleText))
		{
			BoxSlot->SetHorizontalAlignment(HAlign_Center);
			BoxSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 14.f));
		}

		ActivityButtons.Reset();
		ActivityLabels.Reset();
		for (int32 i = 0; i < MaxActivities; ++i)
		{
			UTextBlock* Label = nullptr;
			UButton* Btn = MakeActivityButton(WidgetTree, *FString::Printf(TEXT("ActBtn%d"), i), Label);
			if (UVerticalBoxSlot* BoxSlot = VBox->AddChildToVerticalBox(Btn))
			{
				BoxSlot->SetPadding(FMargin(0.f, 3.f));
				BoxSlot->SetHorizontalAlignment(HAlign_Fill);
			}
			ActivityButtons.Add(Btn);
			ActivityLabels.Add(Label);
		}
		// AddDynamic 宏需编译期函数名 → 逐个显式绑定。
		ActivityButtons[0]->OnClicked.AddDynamic(this, &USGActivityMenuWidget::OnActivity0);
		ActivityButtons[1]->OnClicked.AddDynamic(this, &USGActivityMenuWidget::OnActivity1);
		ActivityButtons[2]->OnClicked.AddDynamic(this, &USGActivityMenuWidget::OnActivity2);
		ActivityButtons[3]->OnClicked.AddDynamic(this, &USGActivityMenuWidget::OnActivity3);
		ActivityButtons[4]->OnClicked.AddDynamic(this, &USGActivityMenuWidget::OnActivity4);
		ActivityButtons[5]->OnClicked.AddDynamic(this, &USGActivityMenuWidget::OnActivity5);
		ActivityButtons[6]->OnClicked.AddDynamic(this, &USGActivityMenuWidget::OnActivity6);
		ActivityButtons[7]->OnClicked.AddDynamic(this, &USGActivityMenuWidget::OnActivity7);
		ActivityButtons[8]->OnClicked.AddDynamic(this, &USGActivityMenuWidget::OnActivity8);

		StatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StatusText"));
		StatusText->SetJustification(ETextJustify::Center);
		StatusText->SetColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.9f, 1.f)));
		if (UVerticalBoxSlot* BoxSlot = VBox->AddChildToVerticalBox(StatusText))
		{
			BoxSlot->SetHorizontalAlignment(HAlign_Center);
			BoxSlot->SetPadding(FMargin(0.f, 12.f, 0.f, 0.f));
		}

		UButton* Close = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("CloseButton"));
		UTextBlock* CloseLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		CloseLabel->SetText(FText::FromString(TEXT("回去")));
		CloseLabel->SetJustification(ETextJustify::Center);
		Close->AddChild(CloseLabel);
		CloseButton = Close;
		if (UVerticalBoxSlot* BoxSlot = VBox->AddChildToVerticalBox(CloseButton))
		{
			BoxSlot->SetPadding(FMargin(0.f, 14.f, 0.f, 0.f));
			BoxSlot->SetHorizontalAlignment(HAlign_Fill);
		}
		CloseButton->OnClicked.AddDynamic(this, &USGActivityMenuWidget::OnCloseClicked);
	}

	return Super::RebuildWidget();
}

UActivitySubsystem* USGActivityMenuWidget::GetActivitySubsystem() const
{
	if (const APlayerController* PC = GetOwningPlayer())
	{
		if (UGameInstance* GI = PC->GetGameInstance())
		{
			return GI->GetSubsystem<UActivitySubsystem>();
		}
	}
	return nullptr;
}

void USGActivityMenuWidget::OpenMenu()
{
	if (!IsInViewport())
	{
		AddToViewport(90);
	}
	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeGameAndUI Mode;
		Mode.SetWidgetToFocus(TakeWidget());
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(Mode);
		PC->SetShowMouseCursor(true);
	}
	Refresh();
}

void USGActivityMenuWidget::CloseMenu()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
	}
	RemoveFromParent();
}

void USGActivityMenuWidget::Refresh()
{
	UActivitySubsystem* Act = GetActivitySubsystem();
	if (!Act) { return; }

	VisibleActivities = Act->GetActivitiesForCurrentLevel();

	for (int32 i = 0; i < ActivityButtons.Num(); ++i)
	{
		const bool bVisible = VisibleActivities.IsValidIndex(i);
		if (ActivityButtons[i])
		{
			ActivityButtons[i]->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
			if (bVisible)
			{
				const bool bCan = Act->CanPerform(VisibleActivities[i]);
				ActivityButtons[i]->SetIsEnabled(bCan);
			}
		}
		if (bVisible && ActivityLabels[i])
		{
			ActivityLabels[i]->SetText(FActivitySystem::GetActivityDef(VisibleActivities[i]).Title);
		}
	}

	// 状态行：当前精力（活动的稀缺资源）。
	if (StatusText)
	{
		int32 Energy = 0;
		if (const APlayerController* PC = GetOwningPlayer())
		{
			if (UGameInstance* GI = PC->GetGameInstance())
			{
				if (const UPlayerStateSubsystem* PS = GI->GetSubsystem<UPlayerStateSubsystem>())
				{
					Energy = PS->GetAttribute(EPlayerAttribute::Energy);
				}
			}
		}
		StatusText->SetText(FText::FromString(FString::Printf(TEXT("当前精力 %d"), Energy)));
	}
}

void USGActivityMenuWidget::Choose(int32 VisibleIndex)
{
	UActivitySubsystem* Act = GetActivitySubsystem();
	if (Act && VisibleActivities.IsValidIndex(VisibleIndex))
	{
		Act->PerformActivity(VisibleActivities[VisibleIndex]);
		Refresh(); // 精力/可用状态变化即时反映
	}
}

void USGActivityMenuWidget::OnActivity0() { Choose(0); }
void USGActivityMenuWidget::OnActivity1() { Choose(1); }
void USGActivityMenuWidget::OnActivity2() { Choose(2); }
void USGActivityMenuWidget::OnActivity3() { Choose(3); }
void USGActivityMenuWidget::OnActivity4() { Choose(4); }
void USGActivityMenuWidget::OnActivity5() { Choose(5); }
void USGActivityMenuWidget::OnActivity6() { Choose(6); }
void USGActivityMenuWidget::OnActivity7() { Choose(7); }
void USGActivityMenuWidget::OnActivity8() { Choose(8); }

void USGActivityMenuWidget::OnCloseClicked()
{
	CloseMenu();
}
