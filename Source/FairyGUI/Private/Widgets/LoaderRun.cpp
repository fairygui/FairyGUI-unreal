#include "Widgets/LoaderRun.h"
#include "Styling/StyleDefaults.h"
#include "Rendering/DrawElements.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Text/DefaultLayoutBlock.h"
#include "Framework/Text/RunUtils.h"
#include "FairyApplication.h"
#include "UI/UIPackage.h"
#include "UI/PackageItem.h"
#include "UI/GLoader.h"

TSharedRef< FLoaderRun > FLoaderRun::Create(UFairyApplication* App, const FHTMLElement& InHTMLElement, const TSharedRef< const FString >& InText, const FTextRange& InRange)
{
    return MakeShareable(new FLoaderRun(App, InHTMLElement, InText, InRange));
}

FLoaderRun::FLoaderRun(UFairyApplication* App, const FHTMLElement& InHTMLElement, const TSharedRef< const FString >& InText, const FTextRange& InRange)
    : Children()
    , HTMLElement(InHTMLElement)
    , Text(InText)
    , Range(InRange)
{
    Loader = NewObject<UGLoader>(App);
    Children.Add(Loader->GetDisplayObject());

    FVector2D SourceSize(0, 0);
    const FString& Src = HTMLElement.Attributes.Get("src");
    if (Src.Len() > 0)
    {
        TSharedPtr<FPackageItem> pii = UUIPackage::GetItemByURL(Src);
        if (pii.IsValid())
            SourceSize = pii->Size;
    }

    Loader->SetURL(Src);

    SourceSize.X = HTMLElement.Attributes.GetInt("width", SourceSize.X);
    SourceSize.Y = HTMLElement.Attributes.GetInt("height", SourceSize.Y);

    if (SourceSize.X == 0)
        SourceSize.X = 5;
    if (SourceSize.Y == 0)
        SourceSize.Y = 5;
    Loader->SetSize(SourceSize);
}

FLoaderRun::~FLoaderRun()
{
}

const TArray< TSharedRef<SWidget> >& FLoaderRun::GetChildren()
{
    return Children;
}

void FLoaderRun::ArrangeChildren(const TSharedRef< ILayoutBlock >& Block, const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
    const float InverseScale = Inverse(AllottedGeometry.Scale);

    ArrangedChildren.AddWidget(
        AllottedGeometry.MakeChild(Children[0], TransformVector(InverseScale, Block->GetSize()), FSlateLayoutTransform(TransformPoint(InverseScale, Block->GetLocationOffset())))
    );
}

int32 FLoaderRun::GetTextIndexAt(const TSharedRef< ILayoutBlock >& Block, const FVector2D& Location, float Scale, ETextHitPoint* const OutHitPoint) const
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

    const FVector2D ScaledImageSize = Loader->GetSize() * Scale;
    const int32 Index = (Location.X <= (Left + (ScaledImageSize.X * 0.5f))) ? Range.BeginIndex : Range.EndIndex;

    if (OutHitPoint)
    {
        const FTextRange BlockRange = Block->GetTextRange();
        const FLayoutBlockTextContext BlockTextContext = Block->GetTextContext();

        // The block for an image will always detect a LTR reading direction, so use the base direction (of the line) for the image hit-point detection
        *OutHitPoint = RunUtils::CalculateTextHitPoint(Index, BlockRange, BlockTextContext.BaseDirection);
    }

    return Index;
}

FVector2D FLoaderRun::GetLocationAt(const TSharedRef< ILayoutBlock >& Block, int32 Offset, float Scale) const
{
    return Block->GetLocationOffset();
}

int32 FLoaderRun::OnPaint(const FPaintArgs& Args, const FTextLayout::FLineView& Line, const TSharedRef< ILayoutBlock >& Block, const FTextBlockStyle& DefaultStyle, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    const float InverseScale = Inverse(AllottedGeometry.Scale);
    const FGeometry WidgetGeometry = AllottedGeometry.MakeChild(TransformVector(InverseScale, Block->GetSize()), FSlateLayoutTransform(TransformPoint(InverseScale, Block->GetLocationOffset())));
    return Children[0]->Paint(Args, WidgetGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

TSharedRef< ILayoutBlock > FLoaderRun::CreateBlock(int32 BeginIndex, int32 EndIndex, FVector2D Size, const FLayoutBlockTextContext& TextContext, const TSharedPtr< IRunRenderer >& Renderer)
{
    return FDefaultLayoutBlock::Create(SharedThis(this), FTextRange(BeginIndex, EndIndex), Size, TextContext, Renderer);
}

int8 FLoaderRun::GetKerning(int32 CurrentIndex, float Scale, const FRunTextContext& TextContext) const
{
    return 0;
}

FVector2D FLoaderRun::Measure(int32 BeginIndex, int32 EndIndex, float Scale, const FRunTextContext& TextContext) const
{
    if (EndIndex - BeginIndex == 0)
    {
        return FVector2D(0, GetMaxHeight(Scale));
    }

    return (Loader->GetSize() + FVector2D(2, 0)) * Scale;
}

int16 FLoaderRun::GetMaxHeight(float Scale) const
{
    return Loader->GetSize().Y * Scale;
}

int16 FLoaderRun::GetBaseLine(float Scale) const
{
    return -Loader->GetSize().Y * 0.2f * Scale;
}

FTextRange FLoaderRun::GetTextRange() const
{
    return Range;
}

void FLoaderRun::SetTextRange(const FTextRange& Value)
{
    Range = Value;
}

void FLoaderRun::Move(const TSharedRef<FString>& NewText, const FTextRange& NewRange)
{
    Text = NewText;
    Range = NewRange;
}

TSharedRef<IRun> FLoaderRun::Clone() const
{
    TSharedRef<FLoaderRun> NewRun = FLoaderRun::Create(Loader->GetApp(), HTMLElement, Text, Range);

    return NewRun;
}

void FLoaderRun::AppendTextTo(FString& AppendToText) const
{
    AppendToText.Append(**Text + Range.BeginIndex, Range.Len());
}

void FLoaderRun::AppendTextTo(FString& AppendToText, const FTextRange& PartialRange) const
{
    check(Range.BeginIndex <= PartialRange.BeginIndex);
    check(Range.EndIndex >= PartialRange.EndIndex);

    AppendToText.Append(**Text + PartialRange.BeginIndex, PartialRange.Len());
}

const FRunInfo& FLoaderRun::GetRunInfo() const
{
    static FRunInfo RunInfo;
    return RunInfo;
}

ERunAttributes FLoaderRun::GetRunAttributes() const
{
    return ERunAttributes::None;
}

void FLoaderRun::AddReferencedObjects(FReferenceCollector& Collector)
{
    Collector.AddReferencedObject(Loader);
}
