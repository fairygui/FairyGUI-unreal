#include "Widgets/STextField.h"
#include "Internationalization/BreakIterator.h"
#include "Utils/HTMLParser.h"
#include "Widgets/LoaderRun.h"
#include "Widgets/BitmapFontRun.h"
#include "UI/GObject.h"
#include "UI/UIPackage.h"

STextField::STextField() :
    bHTML(false),
    AutoSize(EAutoSizeType::None),
    bSingleLine(false),
    MaxWidth(0),
    TextLayout(FSlateTextLayout::Create(this, FTextBlockStyle::GetDefault()))
{
    TextLayout->SetLineBreakIterator(FBreakIterator::CreateCharacterBoundaryIterator());
}

void STextField::Construct(const FArguments& InArgs)
{
    SDisplayObject::Construct(SDisplayObject::FArguments().GObject(InArgs._GObject));
}

void STextField::SetText(const FString& InText, bool bInHTML)
{
    const int32 OldLength = Text.Len();

    // Only compare reasonably sized strings, it's not worth checking this
    // for large blocks of text.
    if (bHTML == bInHTML && OldLength <= 20)
    {
        if (InText.Compare(Text, ESearchCase::CaseSensitive) == 0)
        {
            return;
        }
    }

    Text = InText;
    bHTML = bInHTML;
    TextLayout->DirtyLayout();
    Invalidate(EInvalidateWidget::LayoutAndVolatility);
}

void STextField::SetAutoSize(EAutoSizeType InAutoSize)
{
    if (AutoSize != InAutoSize)
    {
        AutoSize = InAutoSize;
        if (AutoSize == EAutoSizeType::Both)
        {
            TextLayout->SetWrappingWidth(0);
        }
        else
        {
            TextLayout->SetWrappingWidth(Size.X);
        }
    }
}

void STextField::SetSingleLine(bool bInSingleLine)
{
    if (bSingleLine != bInSingleLine)
    {
        bSingleLine = bInSingleLine;
        TextLayout->DirtyLayout();
    }
}

void STextField::SetMaxWidth(float InMaxWidth)
{
    if (MaxWidth != InMaxWidth)
    {
        MaxWidth = InMaxWidth;
        TextLayout->DirtyLayout();
    }
}

FVector2D STextField::GetTextSize()
{
    if(TextLayout->IsLayoutDirty())
        UpdateTextLayout();

    return TextLayout->GetSize();
}

void STextField::SetTextFormat(const FNTextFormat& InFormat)
{
    if (&InFormat != &TextFormat)
        TextFormat = InFormat;
    TextLayout->DirtyLayout();
}

FVector2D STextField::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
    TextLayout->SetScale(LayoutScaleMultiplier);
    if (TextLayout->IsLayoutDirty())
        const_cast<STextField*>(this)->UpdateTextLayout();

    return Size;
}

FChildren* STextField::GetChildren()
{
    return TextLayout->GetChildren();
}

void STextField::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
    TextLayout->ArrangeChildren(AllottedGeometry, ArrangedChildren);
}

int32 STextField::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    FVector2D AutoScrollValue = FVector2D::ZeroVector; // Scroll to the left
    if (TextFormat.Align != EAlignType::Left)
    {
        const float ActualWidth = TextLayout->GetSize().X;
        const float VisibleWidth = Size.X;
        if (VisibleWidth < ActualWidth)
        {
            switch (TextFormat.Align)
            {
            case EAlignType::Center:
                AutoScrollValue.X = (ActualWidth - VisibleWidth) * 0.5f; // Scroll to the center
                break;

            case EAlignType::Right:
                AutoScrollValue.X = (ActualWidth - VisibleWidth); // Scroll to the right
                break;

            default:
                break;
            }
        }
    }

    if (TextFormat.VerticalAlign != EVerticalAlignType::Top)
    {
        const float ActualHeight = TextLayout->GetSize().Y;
        const float VisibleHeight = Size.Y;
        switch (TextFormat.VerticalAlign)
        {
        case EVerticalAlignType::Middle:
            AutoScrollValue.Y = FMath::CeilToFloat((ActualHeight - VisibleHeight) * .5f);
            break;

        case EVerticalAlignType::Bottom:
            AutoScrollValue.Y = FMath::CeilToFloat(ActualHeight - VisibleHeight);
            break;
        }

        if (AutoScrollValue.Y > 0)
            AutoScrollValue.Y = 0;
    }

    TextLayout->SetVisibleRegion(Size, AutoScrollValue * TextLayout->GetScale());
    TextLayout->UpdateIfNeeded();

    LayerId = TextLayout->OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, ShouldBeEnabled(bParentEnabled));

    return LayerId;
}

void STextField::UpdateTextLayout()
{
    TextLayout->ClearLines();
    TextLayout->ClearLineHighlights();
    TextLayout->ClearRunRenderers();

    TextLayout->SetDefaultTextStyle(TextFormat.GetStyle());
    TextLayout->SetJustification((ETextJustify::Type)TextFormat.Align);
    TextLayout->SetWrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping);
    if (AutoSize == EAutoSizeType::Both)
        TextLayout->SetWrappingWidth(MaxWidth);
    else
        TextLayout->SetWrappingWidth(MaxWidth != 0 ? FMath::Min(MaxWidth, Size.X) : Size.X);
    TextLayout->SetMargin(FMargin(2, 2));
    TextLayout->SetLineHeightPercentage(1 + (TextFormat.LineSpacing - 3) / TextFormat.Size);

    HTMLElements.Reset();
    if (bHTML)
    {
        FHTMLParser::DefaultParser.Parse(Text, TextFormat, HTMLElements, FHTMLParser::DefaultParseOptions);
    }
    else
    {
        FHTMLElement TextElement;
        TextElement.Type = EHTMLElementType::Text;
        TextElement.Format = TextFormat;
        TextElement.Text = Text;
        HTMLElements.Add(MoveTemp(TextElement));
    }

    BuildLines();

    TextLayout->UpdateIfNeeded();

    if (AutoSize == EAutoSizeType::Both)
    {
        GObject->SetSize(TextLayout->GetSize());
    }
    else if (AutoSize == EAutoSizeType::Height)
    {
        GObject->SetSize(FVector2D(Size.X, TextLayout->GetSize().Y));
    }
}

void STextField::BuildLines()
{
    class FLineHelper
    {
    public:
        TArray<FTextLayout::FNewLineData> Lines;
        FTextLayout::FNewLineData* LastLineData;
        bool bNewLine;

        FLineHelper()
        {
            NewLine();
        }

        TSharedRef<FString>& GetTextRef()
        {
            if (bNewLine)
                NewLine();

            return LastLineData->Text;
        }

        FString& GetText()
        {
            if (bNewLine)
                NewLine();

            return LastLineData->Text.Get();
        }

        TArray<TSharedRef<IRun>>& GetRuns()
        {
            if (bNewLine)
                NewLine();

            return LastLineData->Runs;
        }

    private:
        void NewLine()
        {
            Lines.Emplace(MakeShareable(new FString()), TArray<TSharedRef<IRun>>());
            LastLineData = &Lines.Last();
            bNewLine = false;
        }
    } LineHelper;

    TSharedPtr<FBitmapFont> BitmapFont;
    if (TextFormat.Face.StartsWith("ui://"))
    {
        TSharedPtr<FPackageItem> FontItem = UUIPackage::GetItemByURL(TextFormat.Face);
        if (FontItem.IsValid())
        {
            FontItem->Load();
            BitmapFont = FontItem->BitmapFont;
        }
    }

    TArray<FTextRange> LineRangesBuffer;
    for (int32 ElementIndex = 0; ElementIndex < HTMLElements.Num(); ++ElementIndex)
    {
        const FHTMLElement& Element = HTMLElements[ElementIndex];
        if (Element.Type == EHTMLElementType::Text)
        {
            LineRangesBuffer.Reset();

            FTextBlockStyle TextStyle = Element.Format.GetStyle();

            FTextRange::CalculateLineRangesFromString(Element.Text, LineRangesBuffer);

            for (int32 LineIndex = 0; LineIndex < LineRangesBuffer.Num(); ++LineIndex)
            {
                const FTextRange& LineRange = LineRangesBuffer[LineIndex];
                FString TextBlock = Element.Text.Mid(LineRange.BeginIndex, LineRange.Len());
                if (BitmapFont.IsValid())
                {
                    int32 len = TextBlock.Len();
                    for (int32 CharIndex = 0; CharIndex < len; CharIndex++)
                    {
                        FTextRange ModelRange;
                        ModelRange.BeginIndex = LineHelper.GetText().Len();
                        LineHelper.GetText().AppendChar(TextBlock[CharIndex]);
                        ModelRange.EndIndex = LineHelper.GetText().Len();
                        LineHelper.GetRuns().Add(FBitmapFontRun::Create(LineHelper.GetTextRef(), BitmapFont.ToSharedRef(), ModelRange));
                    }
                }
                else
                {
                    FTextRange ModelRange;
                    ModelRange.BeginIndex = LineHelper.GetText().Len();
                    LineHelper.GetText().Append(TextBlock);
                    ModelRange.EndIndex = LineHelper.GetText().Len();

                    LineHelper.GetRuns().Add(FSlateTextRun::Create(FRunInfo(), LineHelper.GetTextRef(), TextStyle, ModelRange));
                }

                if (LineIndex != LineRangesBuffer.Num() - 1)
                    LineHelper.bNewLine = true;
            }
        }
        else if (Element.Type == EHTMLElementType::Image)
        {
            FTextRange ModelRange;
            ModelRange.BeginIndex = LineHelper.GetText().Len();
            LineHelper.GetText().Append(TEXT("\x200B")); // Zero-Width Breaking Space
            ModelRange.EndIndex = LineHelper.GetText().Len();
            LineHelper.GetRuns().Add(FLoaderRun::Create(GObject->GetApp(), Element, LineHelper.GetTextRef(), ModelRange));
        }
    }

    TextLayout->AddLines(LineHelper.Lines);
}