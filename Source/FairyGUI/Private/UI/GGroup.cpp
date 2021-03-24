#include "UI/GGroup.h"
#include "UI/GComponent.h"
#include "Utils/ByteBuffer.h"
#include "Widgets/SDisplayObject.h"

UGGroup::UGGroup() :
    MainGridIndex(-1),
    MainGridMinSize(10),
    MainChildIndex(-1)
{
    DisplayObject = SNew(SDisplayObject).GObject(this);;
    DisplayObject->SetInteractable(false);
}

UGGroup::~UGGroup()
{
}

void UGGroup::SetLayout(EGroupLayoutType InLayout)
{
    if (Layout != InLayout)
    {
        Layout = InLayout;
        SetBoundsChangedFlag(true);
    }
}

void UGGroup::SetColumnGap(int32 InColumnGap)
{
    if (ColumnGap != InColumnGap)
    {
        ColumnGap = InColumnGap;
        SetBoundsChangedFlag();
    }
}

void UGGroup::SetLineGap(int32 InLineGap)
{
    if (LineGap != InLineGap)
    {
        LineGap = InLineGap;
        SetBoundsChangedFlag();
    }
}

void UGGroup::SetExcludeInvisibles(bool Flag)
{
    if (bExcludeInvisibles != Flag)
    {
        bExcludeInvisibles = Flag;
        SetBoundsChangedFlag();
    }
}
void UGGroup::SetAutoSizeDisabled(bool Flag)
{
    if (bAutoSizeDisabled != Flag)
    {
        bAutoSizeDisabled = Flag;
        SetBoundsChangedFlag();
    }
}

void UGGroup::SetMainGridIndex(int32 InIndex)
{
    if (MainGridIndex != InIndex)
    {
        MainGridIndex = InIndex;
        SetBoundsChangedFlag();
    }
}

void UGGroup::SetMainGridMinSize(int32 InSize)
{
    if (MainGridMinSize != InSize)
    {
        MainGridMinSize = InSize;
        SetBoundsChangedFlag();
    }
}

void UGGroup::SetBoundsChangedFlag(bool bPositionChangedOnly)
{
    if (Updating == 0 && Parent.IsValid())
    {
        if (!bPositionChangedOnly)
            bPercentReady = false;

        if (!bBoundsChanged)
        {
            bBoundsChanged = true;

            if (Layout != EGroupLayoutType::None)
            {
                GetApp()->DelayCall(UpdateBoundsTimerHandle, this, &UGGroup::EnsureBoundsCorrect);
            }
        }
    }
}

void UGGroup::EnsureBoundsCorrect()
{
    if (!Parent.IsValid() || !bBoundsChanged)
        return;

    bBoundsChanged = false;

    if (bAutoSizeDisabled)
        ResizeChildren(FVector2D::ZeroVector);
    else
    {
        HandleLayout();
        UpdateBounds();
    }
}

void UGGroup::UpdateBounds()
{
    int32 cnt = Parent->NumChildren();
    int32 i;
    UGObject* child;
    float ax = FLT_MAX, ay = FLT_MAX;
    float ar = FLT_MIN, ab = FLT_MIN;
    float tmp;
    bool empty = true;

    for (i = 0; i < cnt; i++)
    {
        child = Parent->GetChildAt(i);
        if (child->GetGroup() != this || (bExcludeInvisibles && !child->InternalVisible3()))
            continue;

        tmp = child->GetX();
        if (tmp < ax)
            ax = tmp;
        tmp = child->GetY();
        if (tmp < ay)
            ay = tmp;
        tmp = child->GetX() + child->GetWidth();
        if (tmp > ar)
            ar = tmp;
        tmp = child->GetY() + child->GetHeight();
        if (tmp > ab)
            ab = tmp;

        empty = false;
    }

    float w;
    float h;
    if (!empty)
    {
        Updating |= 1;
        SetPosition(FVector2D(ax, ay));
        Updating &= 2;

        w = ar - ax;
        h = ab - ay;
    }
    else
        w = h = 0;

    if ((Updating & 2) == 0)
    {
        Updating |= 2;
        SetSize(FVector2D(w, h));
        Updating &= 1;
    }
    else
    {
        Updating &= 1;
        ResizeChildren(FVector2D(GetWidth() - w, GetHeight() - h));
    }
}

void UGGroup::HandleLayout()
{
    Updating |= 1;

    if (Layout == EGroupLayoutType::Horizontal)
    {
        float curX = GetX();
        int32 cnt = Parent->NumChildren();
        for (int32 i = 0; i < cnt; i++)
        {
            UGObject* child = Parent->GetChildAt(i);
            if (child->GetGroup() != this)
                continue;
            if (bExcludeInvisibles && !child->InternalVisible3())
                continue;

            child->SetXMin(curX);
            if (child->GetWidth() != 0)
                curX += child->GetWidth() + ColumnGap;
        }
    }
    else if (Layout == EGroupLayoutType::Vertical)
    {
        float curY = GetY();
        int32 cnt = Parent->NumChildren();
        for (int32 i = 0; i < cnt; i++)
        {
            UGObject* child = Parent->GetChildAt(i);
            if (child->GetGroup() != this)
                continue;
            if (bExcludeInvisibles && !child->InternalVisible3())
                continue;

            child->SetYMin(curY);
            if (child->GetHeight() != 0)
                curY += child->GetHeight() + LineGap;
        }
    }

    Updating &= 2;
}

void UGGroup::MoveChildren(const FVector2D& Delta)
{
    if ((Updating & 1) != 0 || !Parent.IsValid())
        return;

    Updating |= 1;

    int32 cnt = Parent->NumChildren();
    for (int32 i = 0; i < cnt; i++)
    {
        UGObject* child = Parent->GetChildAt(i);
        if (child->GetGroup() == this)
        {
            child->SetPosition(child->GetPosition() + Delta);
        }
    }

    Updating &= 2;
}

void UGGroup::ResizeChildren(const FVector2D& Delta)
{
    if (Layout == EGroupLayoutType::None || (Updating & 2) != 0 || !Parent.IsValid())
        return;

    Updating |= 2;

    if (bBoundsChanged)
    {
        bBoundsChanged = false;
        if (!bAutoSizeDisabled)
        {
            UpdateBounds();
            return;
        }
    }

    int32 cnt = Parent->NumChildren();

    if (!bPercentReady)
    {
        bPercentReady = true;
        NumChildren = 0;
        TotalSize = 0;
        MainChildIndex = -1;

        int32 j = 0;
        for (int32 i = 0; i < cnt; i++)
        {
            UGObject* child = Parent->GetChildAt(i);
            if (child->GetGroup() != this)
                continue;

            if (!bExcludeInvisibles || child->InternalVisible3())
            {
                if (j == MainGridIndex)
                    MainChildIndex = i;

                NumChildren++;

                if (Layout == EGroupLayoutType::Horizontal)
                    TotalSize += child->GetWidth();
                else
                    TotalSize += child->GetHeight();
            }

            j++;
        }

        if (MainChildIndex != -1)
        {
            if (Layout == EGroupLayoutType::Horizontal)
            {
                UGObject* child = Parent->GetChildAt(MainChildIndex);
                TotalSize += MainGridMinSize - child->GetWidth();
                child->SizePercentInGroup = MainGridMinSize / TotalSize;
            }
            else
            {
                UGObject* child = Parent->GetChildAt(MainChildIndex);
                TotalSize += MainGridMinSize - child->GetHeight();
                child->SizePercentInGroup = MainGridMinSize / TotalSize;
            }
        }

        for (int32 i = 0; i < cnt; i++)
        {
            UGObject* child = Parent->GetChildAt(i);
            if (child->GetGroup() != this)
                continue;

            if (i == MainChildIndex)
                continue;

            if (TotalSize > 0)
                child->SizePercentInGroup = (Layout == EGroupLayoutType::Horizontal ? child->GetWidth() : child->GetHeight()) / TotalSize;
            else
                child->SizePercentInGroup = 0;
        }
    }

    float remainSize = 0;
    float remainPercent = 1;
    bool priorHandled = false;

    if (Layout == EGroupLayoutType::Horizontal)
    {
        remainSize = GetWidth() - (NumChildren - 1) * ColumnGap;
        if (MainChildIndex != -1 && remainSize >= TotalSize)
        {
            UGObject* child = Parent->GetChildAt(MainChildIndex);
            child->SetSize(FVector2D(remainSize - (TotalSize - MainGridMinSize), child->RawSize.Y + Delta.Y), true);
            remainSize -= child->GetWidth();
            remainPercent -= child->SizePercentInGroup;
            priorHandled = true;
        }

        float curX = GetX();
        for (int32 i = 0; i < cnt; i++)
        {
            UGObject* child = Parent->GetChildAt(i);
            if (child->GetGroup() != this)
                continue;

            if (bExcludeInvisibles && !child->InternalVisible3())
            {
                child->SetSize(FVector2D(child->RawSize.X, child->RawSize.Y + Delta.Y), true);
                continue;
            }

            if (!priorHandled || i != MainChildIndex)
            {
                child->SetSize(FVector2D(FMath::RoundToInt(child->SizePercentInGroup / remainPercent * remainSize), child->RawSize.Y + Delta.Y), true);
                remainPercent -= child->SizePercentInGroup;
                remainSize -= child->GetWidth();
            }

            child->SetXMin(curX);
            if (child->GetWidth() != 0)
                curX += child->GetWidth() + ColumnGap;
        }
    }
    else
    {
        remainSize = GetHeight() - (NumChildren - 1) * LineGap;
        if (MainChildIndex != -1 && remainSize >= TotalSize)
        {
            UGObject* child = Parent->GetChildAt(MainChildIndex);
            child->SetSize(FVector2D(child->RawSize.X + Delta.X, remainSize - (TotalSize - MainGridMinSize)), true);
            remainSize -= child->GetHeight();
            remainPercent -= child->SizePercentInGroup;
            priorHandled = true;
        }

        float curY = GetY();
        for (int32 i = 0; i < cnt; i++)
        {
            UGObject* child = Parent->GetChildAt(i);
            if (child->GetGroup() != this)
                continue;

            if (bExcludeInvisibles && !child->InternalVisible3())
            {
                child->SetSize(FVector2D(child->RawSize.X + Delta.X, child->RawSize.Y), true);
                continue;
            }

            if (!priorHandled || i != MainChildIndex)
            {
                child->SetSize(FVector2D(child->RawSize.X + Delta.X, FMath::RoundToInt(child->SizePercentInGroup / remainPercent * remainSize)), true);
                remainPercent -= child->SizePercentInGroup;
                remainSize -= child->GetHeight();
            }

            child->SetYMin(curY);
            if (child->GetHeight() != 0)
                curY += child->GetHeight() + LineGap;
        }
    }

    Updating &= 1;
}

void UGGroup::HandleAlphaChanged()
{
    UGObject::HandleAlphaChanged();

    if (bUnderConstruct)
        return;

    int32 cnt = Parent->NumChildren();
    for (int32 i = 0; i < cnt; i++)
    {
        UGObject* child = Parent->GetChildAt(i);
        if (child->GetGroup() == this)
            child->SetAlpha(Alpha);
    }
}

void UGGroup::HandleVisibleChanged()
{
    if (!Parent.IsValid())
        return;

    int32 cnt = Parent->NumChildren();
    for (int32 i = 0; i < cnt; i++)
    {
        UGObject* child = Parent->GetChildAt(i);
        if (child->GetGroup() == this)
            child->HandleVisibleChanged();
    }
}

void UGGroup::SetupBeforeAdd(FByteBuffer* Buffer, int32 BeginPos)
{
    UGObject::SetupBeforeAdd(Buffer, BeginPos);

    Buffer->Seek(BeginPos, 5);

    Layout = (EGroupLayoutType)Buffer->ReadByte();
    LineGap = Buffer->ReadInt();
    ColumnGap = Buffer->ReadInt();
    if (Buffer->Version >= 2)
    {
        bExcludeInvisibles = Buffer->ReadBool();
        bAutoSizeDisabled = Buffer->ReadBool();
        MainGridIndex = Buffer->ReadShort();
    }
}

void UGGroup::SetupAfterAdd(FByteBuffer* Buffer, int32 BeginPos)
{
    UGObject::SetupAfterAdd(Buffer, BeginPos);

    if (!bVisible)
        HandleVisibleChanged();
}
