#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SGMinimapWidget.generated.h"

class UCanvasPanel;
class UBorder;
class UImage;
class UTextBlock;

/**
 * 城市小地图（开放城市枢纽，2026-05-24）。纯 C++ UMG，零 BP 资产。
 *
 * 右上角一块俯视图：各可进建筑按城市坐标描点 + 标签，玩家位置一个亮点实时更新。
 * 仅在城市枢纽关卡显示（室内由玩家逻辑控制隐藏）。由 PlayerCharacter 每帧调 UpdatePlayerDot。
 *
 * 见 docs/superpowers/specs/2026-05-24-open-city-hub-design.md。
 */
UCLASS()
class SGLIFESIM_API USGMinimapWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 城市世界坐标的半幅范围（厘米）；世界 [-Extent,Extent] 映射到地图 [0,MapSize]。 */
	static constexpr float WorldHalfExtent = 6500.f;

	/** 小地图边长（像素）。 */
	static constexpr float MapSize = 200.f;

	/** 把一个城市世界坐标映射到小地图内的本地像素坐标。纯函数，可单测。 */
	static FVector2D WorldToMap(const FVector& WorldXY);

	/** 用玩家世界坐标刷新玩家亮点位置。 */
	void UpdatePlayerDot(const FVector& PlayerWorldLocation);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	/** 在地图上放一个点（颜色 / 标签）。Construct 期间调。 */
	void AddDot(const FVector& WorldLoc, const FLinearColor& Color, const FString& Label, bool bIsPlayer);

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> MapCanvas;

	UPROPERTY(Transient)
	TObjectPtr<UImage> PlayerDot;
};
