#include "UI/SGHudWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"

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
	// 纯 native UUserWidget（无 BP 模板）经 CreateWidget 创建时 WidgetTree 可能为空，
	// 这里兜底建一个，否则下面构造控件树会被跳过、整张 HUD 不渲染。
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}

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

		// 钱包行（左列，状态行下方）
		WalletText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("WalletText"));
		if (UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(WalletText))
		{
			CanvasSlot->SetAnchors(FAnchors(0.f, 0.f));
			CanvasSlot->SetAlignment(FVector2D(0.f, 0.f));
			CanvasSlot->SetPosition(FVector2D(32.f, 56.f));
			CanvasSlot->SetAutoSize(true);
		}
		StyleText(WalletText, 20, FLinearColor(0.7f, 1.f, 0.75f));

		// 属性行（左列，钱包行下方）
		StatsText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StatsText"));
		if (UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(StatsText))
		{
			CanvasSlot->SetAnchors(FAnchors(0.f, 0.f));
			CanvasSlot->SetAlignment(FVector2D(0.f, 0.f));
			CanvasSlot->SetPosition(FVector2D(32.f, 86.f));
			CanvasSlot->SetAutoSize(true);
		}
		StyleText(StatsText, 18, FLinearColor(0.85f, 0.9f, 1.f));

		// 进阶行（左列，属性行下方）
		ProgressionText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ProgressionText"));
		if (UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(ProgressionText))
		{
			CanvasSlot->SetAnchors(FAnchors(0.f, 0.f));
			CanvasSlot->SetAlignment(FVector2D(0.f, 0.f));
			CanvasSlot->SetPosition(FVector2D(32.f, 116.f));
			CanvasSlot->SetAutoSize(true);
		}
		StyleText(ProgressionText, 18, FLinearColor(1.f, 0.92f, 0.78f));

		// 成就 toast（顶部居中）
		ToastText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ToastText"));
		if (UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(ToastText))
		{
			CanvasSlot->SetAnchors(FAnchors(0.5f, 0.f));
			CanvasSlot->SetAlignment(FVector2D(0.5f, 0.f));
			CanvasSlot->SetPosition(FVector2D(0.f, 84.f));
			CanvasSlot->SetAutoSize(true);
		}
		StyleText(ToastText, 26, FLinearColor(1.f, 0.85f, 0.35f));

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

		// 提示 / 对话 / toast 初始隐藏
		PromptText->SetVisibility(ESlateVisibility::Collapsed);
		DialogueText->SetVisibility(ESlateVisibility::Collapsed);
		ToastText->SetVisibility(ESlateVisibility::Collapsed);
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

namespace
{
	// 共用：传空则 Collapsed，否则设文本并显示（不挡点击）。
	void ApplyOptionalText(UTextBlock* Text, const FText& InText)
	{
		if (!Text)
		{
			return;
		}
		const bool bEmpty = InText.IsEmpty();
		Text->SetVisibility(bEmpty ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
		if (!bEmpty)
		{
			Text->SetText(InText);
		}
	}
}

void USGHudWidget::SetWalletText(const FText& InText)
{
	ApplyOptionalText(WalletText, InText);
}

void USGHudWidget::SetStatsText(const FText& InText)
{
	ApplyOptionalText(StatsText, InText);
}

void USGHudWidget::SetProgressionText(const FText& InText)
{
	ApplyOptionalText(ProgressionText, InText);
}

void USGHudWidget::ShowAchievementToast(const FText& InText, float HoldSeconds)
{
	if (!ToastText)
	{
		return;
	}
	ToastText->SetText(InText);
	ToastText->SetVisibility(ESlateVisibility::HitTestInvisible);

	if (UWorld* World = GetWorld())
	{
		FTimerDelegate HideDel = FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (ToastText)
			{
				ToastText->SetVisibility(ESlateVisibility::Collapsed);
			}
		});
		World->GetTimerManager().SetTimer(ToastHideTimer, HideDel, FMath::Max(0.5f, HoldSeconds), false);
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
