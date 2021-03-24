#pragma once

#include "SDisplayObject.h"
#include "NTextFormat.h"

class FAIRYGUI_API STextInput : public SDisplayObject
{
public:
    SLATE_BEGIN_ARGS(STextInput) :
        _GObject(nullptr)
    {}
    SLATE_ARGUMENT(UGObject*, GObject)
    SLATE_END_ARGS()

    STextInput();
    void Construct(const FArguments& InArgs);

    void SetPassword(bool bInPassword);
    void SetSingleLine(bool bInSingleLine);
    void SetTextFormat(const FNTextFormat& InTextFormat);
    void SetOnTextChanged(FOnTextChanged Callback);
    void SetOnTextCommitted(FOnTextCommitted Callback);

    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
    virtual FChildren* GetChildren() override;
    virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;

    TSharedRef<class SMultiLineEditableText> Widget;

protected:
    FSimpleSlot ChildSlot;
};