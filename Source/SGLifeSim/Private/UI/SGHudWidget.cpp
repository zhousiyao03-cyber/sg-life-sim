#include "UI/SGHudWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"

namespace
{
	// 统一给文字加描边，等距明亮背景下也读得清
	void StyleText(UTextBlock* Text, int32 FontSize, const FLinearColor& Color)
	{
		if (!Text)
		{
			return;
		}
		FSlateFontInfo Font = Text->GetFont();
		Font.Size = FontSize;
		Text->SetFont(Font);
		Text->SetColorAndOpacity(FSlateColor(Color));

		Text->SetShadowOffset(FVector2D(1.f, 1.f));
		Text->SetShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.85f));
	}
}

TSharedRef<SWidget> USGHudWidget::RebuildWidget()
{
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(GetRootWidget());
	if (!RootCanvas && WidgetTree)
	{
		RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
		WidgetTree->RootWidget = RootCanvas;

		// 顶部状态行（左上）
		StatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StatusText"));
		if (UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(StatusText))
		{
			CanvasSlot->SetAnchors(FAnchors(0.f, 0.f));
			CanvasSlot->SetAlignment(FVector2D(0.f, 0.f));
			CanvasSlot->SetPosition(FVector2D(32.f, 24.f));
			CanvasSlot->SetAutoSize(true);
		}
		StyleText(StatusText, 20, FLinearColor::White);

		// 交互提示（底部居中，略高）
		PromptText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PromptText"));
		if (UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(PromptText))
		{
			CanvasSlot->SetAnchors(FAnchors(0.5f, 1.f));
			CanvasSlot->SetAlignment(FVector2D(0.5f, 1.f));
			CanvasSlot->SetPosition(FVector2D(0.f, -150.f));
			CanvasSlot->SetAutoSize(true);
		}
		StyleText(PromptText, 22, FLinearColor(0.45f, 1.f, 0.45f));

		// 对话气泡（底部居中）
		DialogueText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DialogueText"));
		if (UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(DialogueText))
		{
			CanvasSlot->SetAnchors(FAnchors(0.5f, 1.f));
			CanvasSlot->SetAlignment(FVector2D(0.5f, 1.f));
			CanvasSlot->SetPosition(FVector2D(0.f, -72.f));
			CanvasSlot->SetAutoSize(true);
		}
		StyleText(DialogueText, 24, FLinearColor(1.f, 0.96f, 0.7f));

		// 提示 / 对话初始隐藏
		PromptText->SetVisibility(ESlateVisibility::Collapsed);
		DialogueText->SetVisibility(ESlateVisibility::Collapsed);
	}

	return Super::RebuildWidget();
}

void USGHudWidget::SetStatusText(const FText& InText)
{
	if (StatusText)
	{
		StatusText->SetText(InText);
	}
}

void USGHudWidget::SetPromptText(const FText& InText)
{
	if (!PromptText)
	{
		return;
	}
	const bool bEmpty = InText.IsEmpty();
	PromptText->SetVisibility(bEmpty ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	if (!bEmpty)
	{
		PromptText->SetText(InText);
	}
}

void USGHudWidget::SetDialogueText(const FText& InText)
{
	if (!DialogueText)
	{
		return;
	}
	const bool bEmpty = InText.IsEmpty();
	DialogueText->SetVisibility(bEmpty ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	if (!bEmpty)
	{
		DialogueText->SetText(InText);
	}
}
