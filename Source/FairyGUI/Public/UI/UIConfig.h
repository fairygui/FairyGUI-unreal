#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "FieldTypes.h"
#include "UIConfig.generated.h"

USTRUCT(BlueprintType)
struct FAIRYGUI_API FUIConfig
{
    GENERATED_USTRUCT_BODY()

public:
    static FUIConfig Config;

    FUIConfig();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    FString DefaultFont;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    FString ButtonSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    float ButtonSoundVolumeScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    int32 DefaultScrollStep;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    float DefaultScrollDecelerationRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    bool DefaultScrollTouchEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    bool DefaultScrollBounceEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    EScrollBarDisplayType DefaultScrollBarDisplay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    FString VerticalScrollBar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    FString HorizontalScrollBar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    int32 TouchDragSensitivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    int32 ClickDragSensitivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    int32 TouchScrollSensitivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    int32 DefaultComboBoxVisibleItemCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    FString GlobalModalWaiting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    FColor ModalLayerColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    FString TooltipsWin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    bool BringWindowToFrontOnClick;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    FString WindowModalWaiting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    FString PopupMenu;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    FString PopupMenuSeperator;
};