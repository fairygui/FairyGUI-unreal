#pragma once

#include "SDisplayObject.h"
#include "UI/FieldTypes.h"
#include "NTextFormat.h"
#include "Utils/HTMLElement.h"

class FSlateTextLayout;
class FSlateTextUnderlineLineHighlighter;

class FAIRYGUI_API STextField : public SDisplayObject
{
public:
    SLATE_BEGIN_ARGS(STextField) :
        _GObject(nullptr)
    {}
    SLATE_ARGUMENT(UGObject*, GObject)
        SLATE_END_ARGS()

        STextField();
    void Construct(const FArguments& InArgs);

    const FString& GetText() const { return Text; }
    void SetText(const FString& InText, bool bInHTML = false);

    EAutoSizeType GetAutoSize() const { return AutoSize; };
    void SetAutoSize(EAutoSizeType InAutoSize);

    bool IsSingleLine() const { return bSingleLine; }
    void SetSingleLine(bool InSingleLine);

    FNTextFormat& GetTextFormat() { return TextFormat; }
    void SetTextFormat(const FNTextFormat& InFormat);

    float GetMaxWidth() const { return MaxWidth; }
    void SetMaxWidth(float InMaxWidth);

    FVector2D GetTextSize();

    virtual FChildren* GetChildren() override;
    virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;

protected:
    virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
    virtual bool ComputeVolatility() const override { return true; }
    void UpdateTextLayout();

    void BuildLines();

protected:
    FString Text;
    bool bHTML;
    EAutoSizeType AutoSize;
    bool bSingleLine;
    float MaxWidth;
    FNTextFormat TextFormat;

    TSharedRef<FSlateTextLayout> TextLayout;
    TArray<FHTMLElement> HTMLElements;
};