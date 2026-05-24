#include "UI/SGEndingWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Systems/EndingEvaluator.h"

TSharedRef<SWidget> USGEndingWidget::RebuildWidget()
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

		// 全屏暗幕（铺满锚点）。
		Backdrop = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Backdrop"));
		Backdrop->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.92f));
		if (UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(Backdrop))
		{
			CanvasSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
			CanvasSlot->SetOffsets(FMargin(0.f));
		}

		// 结局标题（居中偏上）。
		TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TitleText"));
		{
			FSlateFontInfo Font = TitleText->GetFont();
			Font.Size = 48;
			TitleText->SetFont(Font);
			TitleText->SetColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.1f, 0.1f)));
			TitleText->SetJustification(ETextJustify::Center);
		}
		if (UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(TitleText))
		{
			CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f));
			CanvasSlot->SetAlignment(FVector2D(0.5f, 1.f));
			CanvasSlot->SetPosition(FVector2D(0.f, -20.f));
			CanvasSlot->SetAutoSize(true);
		}

		// 收尾文案（居中偏下）。
		FlavorText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("FlavorText"));
		{
			FSlateFontInfo Font = FlavorText->GetFont();
			Font.Size = 22;
			FlavorText->SetFont(Font);
			FlavorText->SetColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.85f)));
			FlavorText->SetJustification(ETextJustify::Center);
		}
		if (UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(FlavorText))
		{
			CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f));
			CanvasSlot->SetAlignment(FVector2D(0.5f, 0.f));
			CanvasSlot->SetPosition(FVector2D(0.f, 24.f));
			CanvasSlot->SetAutoSize(true);
		}
	}

	return Super::RebuildWidget();
}

void USGEndingWidget::ShowEnding(EEnding Ending)
{
	if (!IsInViewport())
	{
		AddToViewport(1000); // 盖在所有 HUD 之上
	}
	if (TitleText)
	{
		TitleText->SetText(FEndingEvaluator::GetEndingTitle(Ending));
	}
	if (FlavorText)
	{
		FlavorText->SetText(FEndingEvaluator::GetEndingFlavor(Ending));
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeUIOnly Mode;
		PC->SetInputMode(Mode);
		PC->SetShowMouseCursor(true);
	}
}
