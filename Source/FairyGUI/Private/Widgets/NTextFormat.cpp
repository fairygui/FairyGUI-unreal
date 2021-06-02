#include "Widgets/NTextFormat.h"
#include "UI/UIConfig.h"
#include "UI/UIPackage.h"

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
        const UObject* Font = UUIPackageStatic::Get().Fonts.FindRef(FontFace);
        if (Font != nullptr)
        {
            FSlateFontInfo SlateFont(Font, Size * 0.75f);
            SlateFont.OutlineSettings.OutlineSize = OutlineSize;
            SlateFont.OutlineSettings.OutlineColor = OutlineColor;
            Style.SetFont(SlateFont);
        }
        else
        {
            FSlateFontInfo SlateFont = FCoreStyle::GetDefaultFontStyle(*FontFace, Size * 0.75f);
            SlateFont.OutlineSettings.OutlineSize = OutlineSize;
            SlateFont.OutlineSettings.OutlineColor = OutlineColor;
            Style.SetFont(SlateFont);
        }
    }

    Style.SetColorAndOpacity(FSlateColor(FLinearColor(Color)));
    Style.SetShadowOffset(ShadowOffset);
    Style.SetShadowColorAndOpacity(ShadowColor);

    return MoveTemp(Style);
}