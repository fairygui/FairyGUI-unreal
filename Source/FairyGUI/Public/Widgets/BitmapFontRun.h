#pragma once

#include "Slate.h"
#include "BitmapFont.h"

class FArrangedChildren;
class FPaintArgs;
class FSlateWindowElementList;
struct FTextBlockStyle;
enum class ETextHitPoint : uint8;

class FAIRYGUI_API FBitmapFontRun : public ISlateRun, public TSharedFromThis< FBitmapFontRun >
{
public:

    static TSharedRef< FBitmapFontRun > Create(const TSharedRef< const FString >& InText, const TSharedRef<FBitmapFont>& InFont, const FTextRange& InRange);

public:

    virtual ~FBitmapFontRun();

    virtual FTextRange GetTextRange() const override;
    virtual void SetTextRange(const FTextRange& Value) override;

    virtual int16 GetBaseLine(float Scale) const override;
    virtual int16 GetMaxHeight(float Scale) const override;

    virtual FVector2D Measure(int32 StartIndex, int32 EndIndex, float Scale, const FRunTextContext& TextContext) const override;

    virtual int8 GetKerning(int32 CurrentIndex, float Scale, const FRunTextContext& TextContext) const override;

    virtual TSharedRef< ILayoutBlock > CreateBlock(int32 StartIndex, int32 EndIndex, FVector2D Size, const FLayoutBlockTextContext& TextContext, const TSharedPtr< IRunRenderer >& Renderer) override;

    virtual int32 OnPaint(const FPaintArgs& Args, const FTextLayout::FLineView& Line, const TSharedRef< ILayoutBlock >& Block, const FTextBlockStyle& DefaultStyle, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

    virtual const TArray< TSharedRef<SWidget> >& GetChildren() override;

    virtual void ArrangeChildren(const TSharedRef< ILayoutBlock >& Block, const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;

    virtual int32 GetTextIndexAt(const TSharedRef< ILayoutBlock >& Block, const FVector2D& Location, float Scale, ETextHitPoint* const OutHitPoint = nullptr) const override;

    virtual FVector2D GetLocationAt(const TSharedRef< ILayoutBlock >& Block, int32 Offset, float Scale) const override;

    virtual void BeginLayout() override {}
    virtual void EndLayout() override {}

    virtual void Move(const TSharedRef<FString>& NewText, const FTextRange& NewRange) override;
    virtual TSharedRef<IRun> Clone() const override;

    virtual void AppendTextTo(FString& AppendToText) const override;
    virtual void AppendTextTo(FString& AppendToText, const FTextRange& PartialRange) const override;

    virtual const FRunInfo& GetRunInfo() const override;

    virtual ERunAttributes GetRunAttributes() const override;

protected:

    FBitmapFontRun(const TSharedRef< const FString >& InText, const TSharedRef<FBitmapFont>& InFont, const FTextRange& InRange);

private:
    TSharedRef< const FString > Text;
    FTextRange Range;

    TSharedRef<FBitmapFont> Font;
    FBitmapFont::FGlyph* Glyph;
    FSlateBrush Brush;
};