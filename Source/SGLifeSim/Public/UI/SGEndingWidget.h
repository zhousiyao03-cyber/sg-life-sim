#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Systems/EndingTypes.h"
#include "SGEndingWidget.generated.h"

class UTextBlock;
class UBorder;

/**
 * 结局演出 overlay（纯 C++ UMG）。任何结局选定时（含理智归零强制的「被压垮」）
 * 盖一层暗幕，居中显示结局标题 + 收尾文案，给这段人生一个看得见的落幕。
 */
UCLASS()
class SGLIFESIM_API USGEndingWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 为某结局打开演出（加视口、暗幕、标题+文案、放出鼠标）。 */
	void ShowEnding(EEnding Ending);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UBorder> Backdrop;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> FlavorText;
};
