#include "Widgets/STextInput.h"
#include "Widgets/Text/SlateEditableTextLayout.h"

class SMyTextInput : public SEditableText
{
public:
    void SetTextFormat(const FNTextFormat& InTextFormat)
    {
        this->EditableTextLayout->SetTextStyle(InTextFormat.GetStyle());
    }

    void SetOnTextChanged(FOnTextChanged Callback)
    {
        OnTextChangedCallback = Callback;
    }
};

STextInput::STextInput():
    Widget(SNew(SMyTextInput)),
    ChildSlot(this)
{
}

void STextInput::Construct(const FArguments& InArgs)
{
    SDisplayObject::Construct(SDisplayObject::FArguments().GObject(InArgs._GObject));

    ChildSlot.AttachWidget(Widget);
}

void STextInput::SetTextFormat(const FNTextFormat& InTextFormat)
{
    StaticCastSharedRef<SMyTextInput>(Widget)->SetTextFormat(InTextFormat);
}

void STextInput::SetOnTextChanged(FOnTextChanged Callback)
{
    StaticCastSharedRef<SMyTextInput>(Widget)->SetOnTextChanged(Callback);
}

FChildren* STextInput::GetChildren()
{
    return &ChildSlot;
}

void STextInput::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
    if (ArrangedChildren.Accepts(Widget->GetVisibility()))
        ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(
            Widget, FVector2D::ZeroVector, Size
        ));
}

int32 STextInput::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    FArrangedChildren ArrangedChildren(EVisibility::Visible);
    ArrangeChildren(AllottedGeometry, ArrangedChildren);

    FArrangedWidget& TheChild = ArrangedChildren[0];
    return TheChild.Widget->Paint(Args.WithNewParent(this), TheChild.Geometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, ShouldBeEnabled(bParentEnabled));
}
