#include "Widgets/NTextFormat.h"
#include "UI/UIConfig.h"

FNTextFormat::FNTextFormat() :
    Size(12),
    Color(FColor::White),
    bBold(false),
    bItalic(false),
    bUnderline(false),
    LineSpacing(3),
    LetterSpacing(0),
    Align(EAlignType::Left),
    VerticalAlign(EVerticalAlignType::Top),
    OutlineColor(FColor::Black),
    OutlineSize(0),
    ShadowColor(FColor::Black),
    ShadowOffset(0, 0)
{

}

bool FNTextFormat::EqualStyle(const FNTextFormat& AnotherFormat) const
{
    return Size == AnotherFormat.Size && Color == AnotherFormat.Color
        && bBold == AnotherFormat.bBold && bUnderline == AnotherFormat.bUnderline
        && bItalic == AnotherFormat.bItalic
        && Align == AnotherFormat.Align;
}

FTextBlockStyle FNTextFormat::GetStyle() const
{
    FTextBlockStyle Style;

    const FString& FontFace = Face.IsEmpty() ? FUIConfig::Config.DefaultFont : Face;
    if (!FontFace.StartsWith("ui://"))
    {
        FSlateFontInfo Font = FCoreStyle::GetDefaultFontStyle(*FontFace, Size * 0.75f);
        Font.OutlineSettings.OutlineSize = OutlineSize;
        Font.OutlineSettings.OutlineColor = OutlineColor;
        Style.SetFont(Font);
    }

    Style.SetColorAndOpacity(FSlateColor(FLinearColor(Color)));
    Style.SetShadowOffset(ShadowOffset);
    Style.SetShadowColorAndOpacity(ShadowColor);

    return MoveTemp(Style);
}