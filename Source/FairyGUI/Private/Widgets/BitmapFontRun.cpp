#include "Widgets/BitmapFontRun.h"
#include "Styling/StyleDefaults.h"
#include "Rendering/DrawElements.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Text/DefaultLayoutBlock.h"
#include "Framework/Text/RunUtils.h"

TSharedRef< FBitmapFontRun > FBitmapFontRun::Create(const TSharedRef< const FString >& InText, const TSharedRef<FBitmapFont>& InFont, const FTextRange& InRange)
{
    return MakeShareable(new FBitmapFontRun(InText, InFont, InRange));
}

FBitmapFontRun::FBitmapFontRun(const TSharedRef< const FString >& InText, const TSharedRef<FBitmapFont>& InFont, const FTextRange& InRange)
    : Text(InText)
    , Range(InRange)
    , Font(InFont)
{
    Glyph = Font->Glyphs.Find(Text.Get()[Range.BeginIndex]);
    if (Glyph != nullptr)
    {
        Brush.SetResourceObject(Font->Texture->NativeTexture);
        Brush.SetImageSize(Glyph->Size);
        Brush.SetUVRegion(Glyph->UVRect);
    }
}

FBitmapFontRun::~FBitmapFontRun()
{
}

const TArray< TSharedRef<SWidget> >& FBitmapFontRun::GetChildren()
{
    static TArray< TSharedRef<SWidget> > NoChildren;
    return NoChildren;
}

void FBitmapFontRun::ArrangeChildren(const TSharedRef< ILayoutBlock >& Block, const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
    // no widgets
}

int32 FBitmapFontRun::GetTextIndexAt(const TSharedRef< ILayoutBlock >& Block, const FVector2D& Location, float Scale, ETextHitPoint* const OutHitPoint) const
{
    // An image should always contain a single character (a breaking space)
    check(Range.Len() == 1);

    const FVector2D& BlockOffset = Block->GetLocationOffset();
    const FVector2D& BlockSize = Block->GetSize();

    const float Left = BlockOffset.X;
    const float Top = BlockOffset.Y;
    const float Right = BlockOffset.X + BlockSize.X;
    const float Bottom = BlockOffset.Y + BlockSize.Y;

    const bool ContainsPoint = Location.X >= Left && Location.X < Right && Location.Y >= Top && Location.Y < Bottom;

    if (!ContainsPoint)
    {
        return INDEX_NONE;
    }

    const float ScaledImageSize = Glyph->XAdvance * Scale;
    const int32 Index = (Location.X <= (Left + (ScaledImageSize * 0.5f))) ? Range.BeginIndex : Range.EndIndex;

    if (OutHitPoint)
    {
        const FTextRange BlockRange = Block->GetTextRange();
        const FLayoutBlockTextContext BlockTextContext = Block->GetTextContext();

        // The block for an image will always detect a LTR reading direction, so use the base direction (of the line) for the image hit-point detection
        *OutHitPoint = RunUtils::CalculateTextHitPoint(Index, BlockRange, BlockTextContext.BaseDirection);
    }

    return Index;
}

FVector2D FBitmapFontRun::GetLocationAt(const TSharedRef< ILayoutBlock >& Block, int32 Offset, float Scale) const
{
    return Block->GetLocationOffset();
}

int32 FBitmapFontRun::OnPaint(const FPaintArgs& Args, const FTextLayout::FLineView& Line, const TSharedRef< ILayoutBlock >& Block, const FTextBlockStyle& DefaultStyle, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    if (Glyph == nullptr)
        return LayerId;

    // The block size and offset values are pre-scaled, so we need to account for that when converting the block offsets into paint geometry
    const float InverseScale = Inverse(AllottedGeometry.Scale);

    FLinearColor FinalColorAndOpacity;
    if (Font->bCanTint)
        FinalColorAndOpacity = InWidgetStyle.GetColorAndOpacityTint() * DefaultStyle.ColorAndOpacity.GetSpecifiedColor();
    else
        FinalColorAndOpacity = InWidgetStyle.GetColorAndOpacityTint();
    const ESlateDrawEffect DrawEffects = bParentEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
    FSlateDrawElement::MakeBox(
        OutDrawElements,
        ++LayerId,
        AllottedGeometry.ToPaintGeometry(Glyph->Size, FSlateLayoutTransform(TransformPoint(InverseScale, Block->GetLocationOffset()) + Glyph->Offset)),
        &Brush,
        DrawEffects,
        FinalColorAndOpacity
    );

    return LayerId;
}

TSharedRef< ILayoutBlock > FBitmapFontRun::CreateBlock(int32 BeginIndex, int32 EndIndex, FVector2D Size, const FLayoutBlockTextContext& TextContext, const TSharedPtr< IRunRenderer >& Renderer)
{
    return FDefaultLayoutBlock::Create(SharedThis(this), FTextRange(BeginIndex, EndIndex), Size, TextContext, Renderer);
}

int8 FBitmapFontRun::GetKerning(int32 CurrentIndex, float Scale, const FRunTextContext& TextContext) const
{
    return 0;
}

FVector2D FBitmapFontRun::Measure(int32 BeginIndex, int32 EndIndex, float Scale, const FRunTextContext& TextContext) const
{
    if (Glyph == nullptr || (EndIndex - BeginIndex == 0))
    {
        return FVector2D(0, GetMaxHeight(Scale));
    }

    return FVector2D(Glyph->XAdvance, Glyph->LineHeight) * Scale;
}

int16 FBitmapFontRun::GetMaxHeight(float Scale) const
{
    if (Glyph == nullptr)
        return 0;
    else
        return Glyph->LineHeight * Scale;
}

int16 FBitmapFontRun::GetBaseLine(float Scale) const
{
    return Scale;
}

FTextRange FBitmapFontRun::GetTextRange() const
{
    return Range;
}

void FBitmapFontRun::SetTextRange(const FTextRange& Value)
{
    Range = Value;
}

void FBitmapFontRun::Move(const TSharedRef<FString>& NewText, const FTextRange& NewRange)
{
    Text = NewText;
    Range = NewRange;
}

TSharedRef<IRun> FBitmapFontRun::Clone() const
{
    TSharedRef<FBitmapFontRun> NewRun = FBitmapFontRun::Create(Text, Font, Range);

    return NewRun;
}

void FBitmapFontRun::AppendTextTo(FString& AppendToText) const
{
    AppendToText.Append(**Text + Range.BeginIndex, Range.Len());
}

void FBitmapFontRun::AppendTextTo(FString& AppendToText, const FTextRange& PartialRange) const
{
    check(Range.BeginIndex <= PartialRange.BeginIndex);
    check(Range.EndIndex >= PartialRange.EndIndex);

    AppendToText.Append(**Text + PartialRange.BeginIndex, PartialRange.Len());
}

const FRunInfo& FBitmapFontRun::GetRunInfo() const
{
    static FRunInfo RunInfo;
    return RunInfo;
}

ERunAttributes FBitmapFontRun::GetRunAttributes() const
{
    return ERunAttributes::None;
}
