#pragma once

#include "SDisplayObject.h"
#include "NTexture.h"
#include "NGraphics.h"

class FAIRYGUI_API SShape : public SDisplayObject
{
public:
    SLATE_BEGIN_ARGS(SShape) :
        _GObject(nullptr)
    {}
    SLATE_ARGUMENT(UGObject*, GObject)
    SLATE_END_ARGS()

    SShape();

	void Construct(const FArguments& InArgs);

    FNGraphics Graphics;
public:

	// SWidget overrides
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

};