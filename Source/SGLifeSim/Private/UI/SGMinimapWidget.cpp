#include "UI/SGMinimapWidget.h"
#include "World/LocationRegistry.h"
#include "World/LocationTypes.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

FVector2D USGMinimapWidget::WorldToMap(const FVector& WorldXY)
{
	// 世界 [-Extent, Extent] → 地图 [0, MapSize]。X→右、Y→下（俯视）。
	const float Nx = FMath::Clamp((WorldXY.X / WorldHalfExtent) * 0.5f + 0.5f, 0.f, 1.f);
	const float Ny = FMath::Clamp((WorldXY.Y / WorldHalfExtent) * 0.5f + 0.5f, 0.f, 1.f);
	return FVector2D(Nx * MapSize, Ny * MapSize);
}

TSharedRef<SWidget> USGMinimapWidget::RebuildWidget()
{
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}

	UCanvasPanel* Root = Cast<UCanvasPanel>(GetRootWidget());
	if (!Root && WidgetTree)
	{
		Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("MinimapRoot"));
		WidgetTree->RootWidget = Root;

		// 右上角半透明地图底。
		UBorder* Backdrop = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("MapBackdrop"));
		Backdrop->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.55f));
		if (UCanvasPanelSlot* BSlot = Root->AddChildToCanvas(Backdrop))
		{
			BSlot->SetAnchors(FAnchors(1.f, 0.f)); // 右上角
			BSlot->SetAlignment(FVector2D(1.f, 0.f));
			BSlot->SetPosition(FVector2D(-24.f, 24.f));
			BSlot->SetSize(FVector2D(MapSize, MapSize));
		}

		// 地图内容画布。
		MapCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("MapCanvas"));
		Backdrop->AddChild(MapCanvas);

		// 各可进建筑描点 + 标签。
		for (int32 i = 1; i < (int32)ELocation::Count; ++i)
		{
			const FLocationDef Def = FLocationRegistry::GetLocationDef((ELocation)i);
			if (Def.LevelName.IsNone()) { continue; }
			AddDot(Def.CityLocation, FLinearColor(0.4f, 0.8f, 1.f), Def.DisplayName.ToString(), /*bIsPlayer=*/false);
		}

		// 玩家亮点（最后加，盖在上层）。
		AddDot(FVector::ZeroVector, FLinearColor(1.f, 0.9f, 0.2f), FString(), /*bIsPlayer=*/true);
	}

	return Super::RebuildWidget();
}

void USGMinimapWidget::AddDot(const FVector& WorldLoc, const FLinearColor& Color, const FString& Label, bool bIsPlayer)
{
	if (!MapCanvas) { return; }

	const FVector2D MapPos = WorldToMap(WorldLoc);
	const float DotSize = bIsPlayer ? 12.f : 8.f;

	UImage* Dot = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
	Dot->SetColorAndOpacity(Color);
	if (UCanvasPanelSlot* DSlot = MapCanvas->AddChildToCanvas(Dot))
	{
		DSlot->SetSize(FVector2D(DotSize, DotSize));
		DSlot->SetPosition(MapPos - FVector2D(DotSize * 0.5f, DotSize * 0.5f));
	}

	if (bIsPlayer)
	{
		PlayerDot = Dot;
	}
	else if (!Label.IsEmpty())
	{
		UTextBlock* Txt = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Txt->SetText(FText::FromString(Label));
		{
			FSlateFontInfo Font = Txt->GetFont();
			Font.Size = 9;
			Txt->SetFont(Font);
		}
		Txt->SetColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.9f, 1.f)));
		if (UCanvasPanelSlot* TSlot = MapCanvas->AddChildToCanvas(Txt))
		{
			TSlot->SetPosition(MapPos + FVector2D(6.f, -4.f));
			TSlot->SetAutoSize(true);
		}
	}
}

void USGMinimapWidget::UpdatePlayerDot(const FVector& PlayerWorldLocation)
{
	if (!PlayerDot) { return; }
	if (UCanvasPanelSlot* DSlot = Cast<UCanvasPanelSlot>(PlayerDot->Slot))
	{
		const FVector2D MapPos = WorldToMap(PlayerWorldLocation);
		DSlot->SetPosition(MapPos - FVector2D(6.f, 6.f));
	}
}
