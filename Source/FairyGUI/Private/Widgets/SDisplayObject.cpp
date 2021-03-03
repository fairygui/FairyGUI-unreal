#include "Widgets/SDisplayObject.h"
#include "FairyApplication.h"
#include "Engine/GameViewportClient.h"

FNoChildren SDisplayObject::NoChildrenInstance;
FName SDisplayObject::SDisplayObjectTag("SDisplayObjectTag");

SDisplayObject::SDisplayObject() :
    bVisible(true),
    bInteractable(true),
    bTouchable(true),
    bOpaque(true),
    Size(ForceInit)
{
    SetCanTick(false);
    bCanSupportFocus = false;
}

void SDisplayObject::Construct(const SDisplayObject::FArguments& InArgs)
{
    GObject = InArgs._GObject;
    SetTag(InArgs._Tag);
}

const FVector2D& SDisplayObject::GetPosition() const
{
    if (!GetRenderTransform().IsSet())
        return FVector2D::ZeroVector;
    else
        return GetRenderTransform()->GetTranslation();
}

void SDisplayObject::SetPosition(const FVector2D& InPosition)
{
    if (!GetRenderTransform().IsSet())
        SetRenderTransform(FSlateRenderTransform(InPosition));
    else
        SetRenderTransform(
            FSlateRenderTransform(GetRenderTransform()->GetMatrix(), InPosition));
}

void SDisplayObject::SetX(float InX)
{
    if (!GetRenderTransform().IsSet())
        SetRenderTransform(FSlateRenderTransform(FVector2D(InX, 0)));
    else
        SetRenderTransform(
            FSlateRenderTransform(GetRenderTransform()->GetMatrix(),
                FVector2D(InX, GetRenderTransform()->GetTranslation().Y)));
}

void SDisplayObject::SetY(float InY)
{
    if (!GetRenderTransform().IsSet())
        SetRenderTransform(FSlateRenderTransform(FVector2D(0, InY)));
    else
        SetRenderTransform(
            FSlateRenderTransform(GetRenderTransform()->GetMatrix(),
                FVector2D(GetRenderTransform()->GetTranslation().X, InY)));
}

void SDisplayObject::SetSize(const FVector2D& InSize)
{
    if (Size != InSize)
    {
        Size = InSize;
        Invalidate(EInvalidateWidget::LayoutAndVolatility);
    }
}

FVector2D SDisplayObject::GetSize()
{
    EnsureSizeCorrect();
    return Size;
}

void SDisplayObject::EnsureSizeCorrect()
{

}

void SDisplayObject::SetVisible(bool bInVisible)
{
    if (bVisible != bInVisible)
    {
        bVisible = bInVisible;
        UpdateVisibilityFlags();
    }
}

void SDisplayObject::SetTouchable(bool bInTouchable)
{
    if (bTouchable != bInTouchable)
    {
        bTouchable = bInTouchable;
        UpdateVisibilityFlags();
    }
}

void SDisplayObject::SetOpaque(bool bInOpaque)
{
    if (bOpaque != bInOpaque)
    {
        bOpaque = bInOpaque;
        UpdateVisibilityFlags();
    }
}

void SDisplayObject::SetInteractable(bool bInInteractable)
{
    if (bInteractable != bInInteractable)
    {
        bInteractable = bInInteractable;
        UpdateVisibilityFlags();
    }
}

void SDisplayObject::UpdateVisibilityFlags()
{
    bool HitTestFlag = bInteractable && bTouchable;
    if (!bVisible)
        SetVisibility(EVisibility::Collapsed);
    else if (!HitTestFlag)
        SetVisibility(EVisibility::HitTestInvisible);
    else if (!bOpaque)
        SetVisibility(EVisibility::SelfHitTestInvisible);
    else
        SetVisibility(EVisibility::All);
}

FVector2D SDisplayObject::ComputeDesiredSize(float) const
{
    return Size;
}

FChildren* SDisplayObject::GetChildren()
{
    return &NoChildrenInstance;
}

void SDisplayObject::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
}

int32 SDisplayObject::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    return LayerId;
}

bool SDisplayObject::OnStage() const
{
    if (!UFairyApplication::IsStarted())
        return false;

    TSharedPtr<SWidget> ViewportWidget = UFairyApplication::Get()->GetViewportWidget();
    TSharedPtr<SWidget> Ptr = GetParentWidget();
    while (Ptr.IsValid())
    {
        if (Ptr == ViewportWidget)
            return true;

        Ptr = Ptr->GetParentWidget();
    }
    return false;
}

FReply SDisplayObject::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    return UFairyApplication::Get()->OnWidgetMouseButtonDown(AsShared(), MyGeometry, MouseEvent);
}

FReply SDisplayObject::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    return UFairyApplication::Get()->OnWidgetMouseButtonUp(AsShared(), MyGeometry, MouseEvent);
}

FReply SDisplayObject::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    return UFairyApplication::Get()->OnWidgetMouseMove(AsShared(), MyGeometry, MouseEvent);
}

FReply SDisplayObject::OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    return UFairyApplication::Get()->OnWidgetMouseButtonDoubleClick(AsShared(), MyGeometry, MouseEvent);
}

void SDisplayObject::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    return UFairyApplication::Get()->OnWidgetMouseEnter(AsShared(), MyGeometry, MouseEvent);
}

void SDisplayObject::OnMouseLeave(const FPointerEvent& MouseEvent)
{
    return UFairyApplication::Get()->OnWidgetMouseLeave(AsShared(), MouseEvent);
}

FReply SDisplayObject::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    return UFairyApplication::Get()->OnWidgetMouseWheel(AsShared(), MyGeometry, MouseEvent);
}