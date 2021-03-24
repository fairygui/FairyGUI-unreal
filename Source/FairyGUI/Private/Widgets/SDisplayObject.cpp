#include "Widgets/SDisplayObject.h"
#include "FairyApplication.h"
#include "Engine/GameViewportClient.h"
#include "UI/GObject.h"

bool SDisplayObject::bMindVisibleOnly = false;
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
    else  if (GObject.IsValid() && GObject->GetHitArea() != nullptr)
        Visibility.BindRaw(this, &SDisplayObject::GetVisibilityFlags);
    else if (!bOpaque)
        SetVisibility(EVisibility::SelfHitTestInvisible);
    else
        SetVisibility(EVisibility::All);
}

EVisibility SDisplayObject::GetVisibilityFlags() const
{
    if (!bMindVisibleOnly && GObject.IsValid() && GObject->GetHitArea() != nullptr)
    {
        FVector2D Pos = GObject->GetApp()->GetTouchPosition();
        Pos = GObject->GlobalToLocal(Pos);
        FBox2D ContentRect(FVector2D::ZeroVector, GObject->GetSize());

        if (!ContentRect.IsInside(Pos))
            return EVisibility::HitTestInvisible;

        FVector2D LayoutScaleMultiplier = GObject->GetSize() / GObject->SourceSize;
        if (LayoutScaleMultiplier.ContainsNaN())
            LayoutScaleMultiplier.Set(1, 1);

        if (!GObject->GetHitArea()->HitTest(ContentRect, LayoutScaleMultiplier, Pos))
            return EVisibility::HitTestInvisible;
        else
            return EVisibility::All;
    }
    else
    {
        if (!bOpaque)
            return EVisibility::SelfHitTestInvisible;
        else
            return EVisibility::All;
    }
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

FReply SDisplayObject::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    UGObject* Obj = GetWidgetGObject(AsShared());
    if (Obj != nullptr)
        return Obj->GetApp()->OnWidgetMouseButtonDown(AsShared(), MyGeometry, MouseEvent);
    else
        return FReply::Unhandled();
}

FReply SDisplayObject::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    UGObject* Obj = GetWidgetGObject(AsShared());
    if (Obj != nullptr)
        return Obj->GetApp()->OnWidgetMouseButtonUp(AsShared(), MyGeometry, MouseEvent);
    else
        return FReply::Unhandled();
}

FReply SDisplayObject::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    UGObject* Obj = GetWidgetGObject(AsShared());
    if (Obj != nullptr)
        return Obj->GetApp()->OnWidgetMouseMove(AsShared(), MyGeometry, MouseEvent);
    else
        return FReply::Unhandled();
}

FReply SDisplayObject::OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    UGObject* Obj = GetWidgetGObject(AsShared());
    if (Obj != nullptr)
        return Obj->GetApp()->OnWidgetMouseButtonDoubleClick(AsShared(), MyGeometry, MouseEvent);
    else
        return FReply::Unhandled();
}

void SDisplayObject::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    UGObject* Obj = GetWidgetGObject(AsShared());
    if (Obj != nullptr)
        Obj->GetApp()->OnWidgetMouseEnter(AsShared(), MyGeometry, MouseEvent);
}

void SDisplayObject::OnMouseLeave(const FPointerEvent& MouseEvent)
{
    UGObject* Obj = GetWidgetGObject(AsShared());
    if (Obj != nullptr)
        Obj->GetApp()->OnWidgetMouseLeave(AsShared(), MouseEvent);
}

FReply SDisplayObject::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    UGObject* Obj = GetWidgetGObject(AsShared());
    if (Obj != nullptr)
        return Obj->GetApp()->OnWidgetMouseWheel(AsShared(), MyGeometry, MouseEvent);
    else
        return FReply::Unhandled();
}

bool SDisplayObject::IsWidgetOnStage(const TSharedPtr<SWidget>& InWidget)
{
    TSharedPtr<SWidget> Ptr = InWidget;
    while (Ptr.IsValid())
    {
        if (Ptr->Advanced_IsWindow())
            return true;

        Ptr = Ptr->GetParentWidget();
    }

    return false;
}

UGObject* SDisplayObject::GetWidgetGObject(const TSharedPtr<SWidget>& InWidget)
{
    TSharedPtr<SWidget> Ptr = InWidget;
    while (Ptr.IsValid() && !Ptr->Advanced_IsWindow())
    {
        if (Ptr->GetTag() == SDisplayObject::SDisplayObjectTag)
        {
            const TWeakObjectPtr<UGObject>& ObjPtr = StaticCastSharedPtr<SDisplayObject>(Ptr)->GObject;
            if (ObjPtr.IsValid())
                return ObjPtr.Get();
        }

        Ptr = Ptr->GetParentWidget();
    }

    return nullptr;
}

UGObject* SDisplayObject::GetWidgetGObjectIfOnStage(const TSharedPtr<SWidget>& InWidget)
{
    TSharedPtr<SWidget> Ptr = InWidget;
    UGObject* Result = nullptr;
    while (Ptr.IsValid())
    {
        if (Ptr->Advanced_IsWindow())
            return Result;

        if (Result == nullptr && Ptr->GetTag() == SDisplayObject::SDisplayObjectTag)
            Result = StaticCastSharedPtr<SDisplayObject>(Ptr)->GObject.Get();

        Ptr = Ptr->GetParentWidget();
    }

    return nullptr;
}

void SDisplayObject::GetWidgetPathToRoot(const TSharedRef<SWidget>& InWidget, TArray<UGObject*>& OutArray)
{
    TSharedPtr<SWidget> Ptr = InWidget;
    while (Ptr.IsValid() && !Ptr->Advanced_IsWindow())
    {
        if (Ptr->GetTag() == SDisplayObject::SDisplayObjectTag)
        {
            const TWeakObjectPtr<UGObject>& ObjPtr = StaticCastSharedPtr<SDisplayObject>(Ptr)->GObject;
            if (ObjPtr.IsValid())
                OutArray.Add(ObjPtr.Get());
        }

        Ptr = Ptr->GetParentWidget();
    }
}

void SDisplayObject::GetWidgetDescendants(const TSharedRef<SWidget>& InWidget, TArray<UGObject*>& OutArray)
{
    if (InWidget->GetTag() == SDisplayObject::SDisplayObjectTag)
    {
        const TSharedRef<SDisplayObject>& DisplayObject = StaticCastSharedRef<SDisplayObject>(InWidget);
        if (DisplayObject->GObject.IsValid())
            OutArray.Add(DisplayObject->GObject.Get());
    }

    FChildren* Children = InWidget->GetChildren();
    for (int32 SlotIdx = 0; SlotIdx < Children->Num(); ++SlotIdx)
    {
        GetWidgetDescendants(Children->GetChildAt(SlotIdx), OutArray);
    }
}