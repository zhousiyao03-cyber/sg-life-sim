#include "UI/SGDialogueWidget.h"

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
#include "Systems/DialogueSubsystem.h"

TSharedRef<SWidget> USGDialogueWidget::RebuildWidget()
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

		// 底部对话面板：半透明黑底，锚定底边、左右留边（避开窗口化 PIE 右边裁切坑）。
		UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Panel"));
		Panel->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.78f));
		Panel->SetPadding(FMargin(28.f, 20.f));
		if (UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(Panel))
		{
			// 锚到底部一条：左 0.04 → 右 0.96，贴底，固定高度由内容撑开。
			PanelSlot->SetAnchors(FAnchors(0.04f, 1.0f, 0.96f, 1.0f));
			PanelSlot->SetAlignment(FVector2D(0.f, 1.f));
			PanelSlot->SetOffsets(FMargin(0.f, 0.f, 0.f, 36.f)); // 离底 36px
			PanelSlot->SetAutoSize(true);
		}

		UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("VBox"));
		Panel->AddChild(VBox);

		SpeakerText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SpeakerText"));
		SpeakerText->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.85f, 0.45f)));
		{
			FSlateFontInfo Font = SpeakerText->GetFont();
			Font.Size = 22;
			SpeakerText->SetFont(Font);
		}
		if (UVerticalBoxSlot* BoxSlot = VBox->AddChildToVerticalBox(SpeakerText))
		{
			BoxSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 6.f));
		}

		LineText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LineText"));
		LineText->SetAutoWrapText(true);
		{
			FSlateFontInfo Font = LineText->GetFont();
			Font.Size = 20;
			LineText->SetFont(Font);
		}
		if (UVerticalBoxSlot* BoxSlot = VBox->AddChildToVerticalBox(LineText))
		{
			BoxSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 14.f));
		}

		ChoicesBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ChoicesBox"));
		VBox->AddChildToVerticalBox(ChoicesBox);

		// 预建固定数量的选项按钮，刷新时只改文本 / 可见性。
		ChoiceButtons.Reset();
		ChoiceLabels.Reset();
		for (int32 Index = 0; Index < MaxChoices; ++Index)
		{
			UButton* Button = WidgetTree->ConstructWidget<UButton>(
				UButton::StaticClass(), *FString::Printf(TEXT("ChoiceButton%d"), Index));

			UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(
				UTextBlock::StaticClass(), *FString::Printf(TEXT("ChoiceLabel%d"), Index));
			Label->SetJustification(ETextJustify::Center);
			{
				FSlateFontInfo Font = Label->GetFont();
				Font.Size = 18;
				Label->SetFont(Font);
			}
			Button->AddChild(Label);

			if (UVerticalBoxSlot* BoxSlot = ChoicesBox->AddChildToVerticalBox(Button))
			{
				BoxSlot->SetPadding(FMargin(0.f, 3.f));
				BoxSlot->SetHorizontalAlignment(HAlign_Fill);
			}

			ChoiceButtons.Add(Button);
			ChoiceLabels.Add(Label);
		}

		// AddDynamic 宏在编译期 stringify 函数名，不能用运行时函数指针数组 → 逐个显式绑定。
		ChoiceButtons[0]->OnClicked.AddDynamic(this, &USGDialogueWidget::OnChoice0);
		ChoiceButtons[1]->OnClicked.AddDynamic(this, &USGDialogueWidget::OnChoice1);
		ChoiceButtons[2]->OnClicked.AddDynamic(this, &USGDialogueWidget::OnChoice2);
		ChoiceButtons[3]->OnClicked.AddDynamic(this, &USGDialogueWidget::OnChoice3);
		ChoiceButtons[4]->OnClicked.AddDynamic(this, &USGDialogueWidget::OnChoice4);
		ChoiceButtons[5]->OnClicked.AddDynamic(this, &USGDialogueWidget::OnChoice5);
	}

	return Super::RebuildWidget();
}

UDialogueSubsystem* USGDialogueWidget::GetDialogueSubsystem() const
{
	if (const APlayerController* PC = GetOwningPlayer())
	{
		if (UGameInstance* GI = PC->GetGameInstance())
		{
			return GI->GetSubsystem<UDialogueSubsystem>();
		}
	}
	return nullptr;
}

bool USGDialogueWidget::OpenForTree(FName TreeId)
{
	UDialogueSubsystem* Dialogue = GetDialogueSubsystem();
	if (!Dialogue || !Dialogue->StartDialogue(TreeId))
	{
		return false;
	}

	if (!bSubscribed)
	{
		Dialogue->OnDialogueChanged.AddUniqueDynamic(this, &USGDialogueWidget::HandleDialogueChanged);
		bSubscribed = true;
	}

	if (!IsInViewport())
	{
		AddToViewport(80); // HUD 之上、菜单(100)之下
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
	return true;
}

void USGDialogueWidget::Close()
{
	if (bSubscribed)
	{
		if (UDialogueSubsystem* Dialogue = GetDialogueSubsystem())
		{
			Dialogue->OnDialogueChanged.RemoveDynamic(this, &USGDialogueWidget::HandleDialogueChanged);
		}
		bSubscribed = false;
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
	}
	RemoveFromParent();
}

void USGDialogueWidget::HandleDialogueChanged()
{
	UDialogueSubsystem* Dialogue = GetDialogueSubsystem();
	if (!Dialogue || !Dialogue->IsDialogueActive())
	{
		Close();
		return;
	}
	Refresh();
}

void USGDialogueWidget::Refresh()
{
	UDialogueSubsystem* Dialogue = GetDialogueSubsystem();
	if (!Dialogue)
	{
		return;
	}

	if (SpeakerText) { SpeakerText->SetText(Dialogue->GetCurrentSpeaker()); }
	if (LineText) { LineText->SetText(Dialogue->GetCurrentLine()); }

	const TArray<FText> Choices = Dialogue->GetChoiceTexts();
	for (int32 Index = 0; Index < ChoiceButtons.Num(); ++Index)
	{
		const bool bVisible = Choices.IsValidIndex(Index);
		if (ChoiceButtons[Index])
		{
			ChoiceButtons[Index]->SetVisibility(
				bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		}
		if (bVisible && ChoiceLabels[Index])
		{
			ChoiceLabels[Index]->SetText(Choices[Index]);
		}
	}
}

void USGDialogueWidget::Choose(int32 VisibleIndex)
{
	if (UDialogueSubsystem* Dialogue = GetDialogueSubsystem())
	{
		// ChooseOption 内部会 Broadcast OnDialogueChanged → HandleDialogueChanged 刷新/关闭。
		Dialogue->ChooseOption(VisibleIndex);
	}
}

void USGDialogueWidget::OnChoice0() { Choose(0); }
void USGDialogueWidget::OnChoice1() { Choose(1); }
void USGDialogueWidget::OnChoice2() { Choose(2); }
void USGDialogueWidget::OnChoice3() { Choose(3); }
void USGDialogueWidget::OnChoice4() { Choose(4); }
void USGDialogueWidget::OnChoice5() { Choose(5); }
