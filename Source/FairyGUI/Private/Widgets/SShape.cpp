#include "Widgets/SShape.h"
#include "Widgets/NTexture.h"
#include "Widgets/Mesh/VertexHelper.h"

SShape::SShape()
{
}

void SShape::Construct(const FArguments& InArgs)
{
    SDisplayObject::Construct(SDisplayObject::FArguments().GObject(InArgs._GObject));
    Graphics.SetTexture(UNTexture::GetWhiteTexture());
}

int32 SShape::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    const bool bIsEnabled = ShouldBeEnabled(bParentEnabled);
    const_cast<SShape*>(this)->Graphics.Paint(AllottedGeometry, OutDrawElements, LayerId,
        InWidgetStyle.GetColorAndOpacityTint().A, bIsEnabled);
    return LayerId;
}