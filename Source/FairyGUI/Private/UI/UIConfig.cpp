#include "UI/UIConfig.h"
#include "FairyApplication.h"

FUIConfig FUIConfig::Config;

FUIConfig::FUIConfig() :
    ButtonSoundVolumeScale(1),
    DefaultScrollStep(25),
    DefaultScrollDecelerationRate(0.967f),
    DefaultScrollTouchEffect(true),
    DefaultScrollBounceEffect(true),
    DefaultScrollBarDisplay(EScrollBarDisplayType::Default),
    TouchDragSensitivity(10),
    ClickDragSensitivity(2),
    TouchScrollSensitivity(20),
    DefaultComboBoxVisibleItemCount(10),
    ModalLayerColor(0, 0, 0, 120),
    BringWindowToFrontOnClick(true)
{
}