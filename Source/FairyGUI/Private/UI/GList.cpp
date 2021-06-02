#include "UI/GList.h"
#include "UI/GButton.h"
#include "UI/GObjectPool.h"
#include "UI/GController.h"
#include "UI/GScrollBar.h"
#include "UI/UIPackage.h"
#include "Utils/ByteBuffer.h"
#include "Widgets/SContainer.h"
#include "FairyApplication.h"

UGList::FItemInfo::FItemInfo() :
    Obj(nullptr),
    UpdateFlag(0),
    bSelected(false)
{
}

UGList::UGList() :
    bScrollItemToViewOnClick(true),
    bAutoResizeItem(true),
    LastSelectedIndex(-1),
    FirstIndex(-1)
{
    bTrackBounds = true;
    SetOpaque(true);
    Pool = new FGObjectPool();
}

UGList::~UGList()
{
    delete Pool;

    SelectionController = nullptr;
    bScrollItemToViewOnClick = false;
}

void UGList::SetDefaultItem(const FString& InDefaultItem)
{
    DefaultItem = UUIPackage::NormalizeURL(InDefaultItem);
}

void UGList::SetLayout(EListLayoutType InLayout)
{
    if (Layout != InLayout)
    {
        Layout = InLayout;
        SetBoundsChangedFlag();
        if (bVirtual)
            SetVirtualListChangedFlag(true);
    }
}

void UGList::SetLineCount(int32 InLineCount)
{
    if (LineCount != InLineCount)
    {
        LineCount = InLineCount;
        if (Layout == EListLayoutType::FlowVertical || Layout == EListLayoutType::Pagination)
        {
            SetBoundsChangedFlag();
            if (bVirtual)
                SetVirtualListChangedFlag(true);
        }
    }
}

void UGList::SetColumnCount(int32 InColumnCount)
{
    if (ColumnCount != InColumnCount)
    {
        ColumnCount = InColumnCount;
        if (Layout == EListLayoutType::FlowHorizontal || Layout == EListLayoutType::Pagination)
        {
            SetBoundsChangedFlag();
            if (bVirtual)
                SetVirtualListChangedFlag(true);
        }
    }
}

void UGList::SetLineGap(int32 InLineGap)
{
    if (LineGap != InLineGap)
    {
        LineGap = InLineGap;
        SetBoundsChangedFlag();
        if (bVirtual)
            SetVirtualListChangedFlag(true);
    }
}

void UGList::SetColumnGap(int32 InColumnGap)
{
    if (ColumnGap != InColumnGap)
    {
        ColumnGap = InColumnGap;
        SetBoundsChangedFlag();
        if (bVirtual)
            SetVirtualListChangedFlag(true);
    }
}

void UGList::SetAlign(EAlignType InAlign)
{
    if (Align != InAlign)
    {
        Align = InAlign;
        SetBoundsChangedFlag();
        if (bVirtual)
            SetVirtualListChangedFlag(true);
    }
}

void UGList::SetVerticalAlign(EVerticalAlignType InVerticalAlign)
{
    if (VerticalAlign != InVerticalAlign)
    {
        VerticalAlign = InVerticalAlign;
        SetBoundsChangedFlag();
        if (bVirtual)
            SetVirtualListChangedFlag(true);
    }
}

void UGList::SetAutoResizeItem(bool bFlag)
{
    if (bAutoResizeItem != bFlag)
    {
        bAutoResizeItem = bFlag;
        SetBoundsChangedFlag();
        if (bVirtual)
            SetVirtualListChangedFlag(true);
    }
}

UGObject* UGList::GetFromPool()
{
    return GetFromPool(G_EMPTY_STRING);
}

UGObject* UGList::GetFromPool(const FString& URL)
{
    UGObject* ret;
    if (URL.Len() == 0)
        ret = Pool->GetObject(DefaultItem, this);
    else
        ret = Pool->GetObject(URL, this);
    if (ret != nullptr)
        ret->SetVisible(true);
    return ret;
}

void UGList::ReturnToPool(UGObject* Obj)
{
    Pool->ReturnObject(Obj);
}

UGObject* UGList::AddItemFromPool(const FString& URL)
{
    UGObject* Obj = GetFromPool(URL);

    return AddChild(Obj);
}

UGObject* UGList::AddChildAt(UGObject* Child, int32 Index)
{
    UGComponent::AddChildAt(Child, Index);
    if (Child->IsA<UGButton>())
    {
        UGButton* Button = (UGButton*)Child;
        Button->SetSelected(false);
        Button->bChangeStateOnClick = false;
    }

    Child->OnClick.AddUniqueDynamic(this, &UGList::OnClickItemHandler);

    return Child;
}

void UGList::RemoveChildAt(int32 Index)
{
    UGObject* Child = Children[Index];
    Child->OnClick.RemoveDynamic(this, &UGList::OnClickItemHandler);

    UGComponent::RemoveChildAt(Index);
}

void UGList::RemoveChildToPoolAt(int32 Index)
{
    ReturnToPool(GetChildAt(Index));
    RemoveChildAt(Index);
}

void UGList::RemoveChildToPool(UGObject* Child)
{
    ReturnToPool(Child);
    RemoveChild(Child);
}

void UGList::RemoveChildrenToPool(int32 BeginIndex, int32 EndIndex)
{
    if (EndIndex < 0 || EndIndex >= Children.Num())
        EndIndex = Children.Num() - 1;

    for (int32 i = BeginIndex; i <= EndIndex; ++i)
        RemoveChildToPoolAt(BeginIndex);
}

int32 UGList::GetSelectedIndex() const
{
    if (bVirtual)
    {
        int32 cnt = RealNumItems;
        for (int32 i = 0; i < cnt; i++)
        {
            const FItemInfo& ii = VirtualItems[i];
            if ((Cast<UGButton>(ii.Obj) && ((UGButton*)ii.Obj)->IsSelected()) || (ii.Obj == nullptr && ii.bSelected))
            {
                if (bLoop)
                    return i % NumItems;
                else
                    return i;
            }
        }
    }
    else
    {
        int32 cnt = Children.Num();
        for (int32 i = 0; i < cnt; i++)
        {
            UGButton* Obj = Cast<UGButton>(Children[i]);
            if (Obj != nullptr && Obj->IsSelected())
                return i;
        }
    }
    return -1;
}

void UGList::SetSelectedIndex(int32 Index)
{
    if (Index >= 0 && Index < GetNumItems())
    {
        if (SelectionMode != EListSelectionMode::Single)
            ClearSelection();
        AddSelection(Index, false);
    }
    else
        ClearSelection();
}

void UGList::SetSelectionController(UGController* InController)
{
    SelectionController = InController;
}

void UGList::GetSelection(TArray<int32>& OutIndice) const
{
    OutIndice.Reset();
    if (bVirtual)
    {
        int32 cnt = RealNumItems;
        for (int32 i = 0; i < cnt; i++)
        {
            const FItemInfo& ii = VirtualItems[i];
            if ((Cast<UGButton>(ii.Obj) && ((UGButton*)ii.Obj)->IsSelected()) || (ii.Obj == nullptr && ii.bSelected))
            {
                int32 j = i;
                if (bLoop)
                {
                    j = i % NumItems;
                    if (OutIndice.Contains(j))
                        continue;
                }
                OutIndice.Add(j);
            }
        }
    }
    else
    {
        int32 cnt = Children.Num();
        for (int32 i = 0; i < cnt; i++)
        {
            UGButton* Obj = Cast<UGButton>(Children[i]);
            if (Obj != nullptr && Obj->IsSelected())
                OutIndice.Add(i);
        }
    }
}

void UGList::AddSelection(int32 Index, bool bScrollItToView)
{
    if (SelectionMode == EListSelectionMode::None)
        return;

    CheckVirtualList();

    if (SelectionMode == EListSelectionMode::Single)
        ClearSelection();

    if (bScrollItToView)
        ScrollToView(Index);

    LastSelectedIndex = Index;
    UGButton* Obj = nullptr;
    if (bVirtual)
    {
        FItemInfo& ii = VirtualItems[Index];
        if (ii.Obj != nullptr)
            Obj = ii.Obj->As<UGButton>();
        ii.bSelected = true;
    }
    else
        Obj = GetChildAt(Index)->As<UGButton>();

    if (Obj != nullptr && !Obj->IsSelected())
    {
        Obj->SetSelected(true);
        UpdateSelectionController(Index);
    }
}

void UGList::RemoveSelection(int32 Index)
{
    if (SelectionMode == EListSelectionMode::None)
        return;

    UGButton* Obj = nullptr;
    if (bVirtual)
    {
        FItemInfo& ii = VirtualItems[Index];
        if (ii.Obj != nullptr)
            Obj = ii.Obj->As<UGButton>();
        ii.bSelected = false;
    }
    else
        Obj = GetChildAt(Index)->As<UGButton>();

    if (Obj != nullptr)
        Obj->SetSelected(false);
}

void UGList::ClearSelection()
{
    if (bVirtual)
    {
        int32 cnt = RealNumItems;
        for (int32 i = 0; i < cnt; i++)
        {
            FItemInfo& ii = VirtualItems[i];
            if (Cast<UGButton>(ii.Obj))
                ((UGButton*)ii.Obj)->SetSelected(false);
            ii.bSelected = false;
        }
    }
    else
    {
        int32 cnt = Children.Num();
        for (int32 i = 0; i < cnt; i++)
        {
            UGButton* Obj = Children[i]->As<UGButton>();
            if (Obj != nullptr)
                Obj->SetSelected(false);
        }
    }
}

void UGList::ClearSelectionExcept(UGObject* Obj)
{
    if (bVirtual)
    {
        int32 cnt = RealNumItems;
        for (int32 i = 0; i < cnt; i++)
        {
            FItemInfo& ii = VirtualItems[i];
            if (ii.Obj != Obj)
            {
                if (Cast<UGButton>(ii.Obj))
                    ((UGButton*)ii.Obj)->SetSelected(false);
                ii.bSelected = false;
            }
        }
    }
    else
    {
        int32 cnt = Children.Num();
        for (int32 i = 0; i < cnt; i++)
        {
            UGButton* Child = Children[i]->As<UGButton>();
            if (Child != nullptr && Child != Obj)
                Child->SetSelected(false);
        }
    }
}

void UGList::SelectAll()
{
    CheckVirtualList();

    int32 last = -1;
    if (bVirtual)
    {
        int32 cnt = RealNumItems;
        for (int32 i = 0; i < cnt; i++)
        {
            FItemInfo& ii = VirtualItems[i];
            if (Cast<UGButton>(ii.Obj) && !((UGButton*)ii.Obj)->IsSelected())
            {
                ((UGButton*)ii.Obj)->SetSelected(true);
                last = i;
            }
            ii.bSelected = true;
        }
    }
    else
    {
        int32 cnt = Children.Num();
        for (int32 i = 0; i < cnt; i++)
        {
            UGButton* Obj = Children[i]->As<UGButton>();
            if (Obj != nullptr && !Obj->IsSelected())
            {
                Obj->SetSelected(true);
                last = i;
            }
        }
    }

    if (last != -1)
        UpdateSelectionController(last);
}

void UGList::SelectReverse()
{
    CheckVirtualList();

    int32 last = -1;
    if (bVirtual)
    {
        int32 cnt = RealNumItems;
        for (int32 i = 0; i < cnt; i++)
        {
            FItemInfo& ii = VirtualItems[i];
            if (Cast<UGButton>(ii.Obj))
            {
                ((UGButton*)ii.Obj)->SetSelected(!((UGButton*)ii.Obj)->IsSelected());
                if (((UGButton*)ii.Obj)->IsSelected())
                    last = i;
            }
            ii.bSelected = !ii.bSelected;
        }
    }
    else
    {
        int32 cnt = Children.Num();
        for (int32 i = 0; i < cnt; i++)
        {
            UGButton* Obj = Children[i]->As<UGButton>();
            if (Obj != nullptr)
            {
                Obj->SetSelected(!Obj->IsSelected());
                if (Obj->IsSelected())
                    last = i;
            }
        }
    }

    if (last != -1)
        UpdateSelectionController(last);
}

void UGList::HandleArrowKey(int32 Direction)
{
    int32 index = GetSelectedIndex();
    if (index == -1)
        return;

    switch (Direction)
    {
    case 1: //up
        if (Layout == EListLayoutType::SingleColumn || Layout == EListLayoutType::FlowVertical)
        {
            index--;
            if (index >= 0)
            {
                ClearSelection();
                AddSelection(index, true);
            }
        }
        else if (Layout == EListLayoutType::FlowHorizontal || Layout == EListLayoutType::Pagination)
        {
            UGObject* current = Children[index];
            int32 k = 0;
            int32 i;
            for (i = index - 1; i >= 0; i--)
            {
                UGObject* obj = Children[i];
                if (obj->GetY() != current->GetY())
                {
                    current = obj;
                    break;
                }
                k++;
            }
            for (; i >= 0; i--)
            {
                UGObject* obj = Children[i];
                if (obj->GetY() != current->GetY())
                {
                    ClearSelection();
                    AddSelection(i + k + 1, true);
                    break;
                }
            }
        }
        break;

    case 3: //right
        if (Layout == EListLayoutType::SingleRow || Layout == EListLayoutType::FlowHorizontal || Layout == EListLayoutType::Pagination)
        {
            index++;
            if (index < Children.Num())
            {
                ClearSelection();
                AddSelection(index, true);
            }
        }
        else if (Layout == EListLayoutType::FlowVertical)
        {
            UGObject* current = Children[index];
            int32 k = 0;
            int32 cnt = Children.Num();
            int32 i;
            for (i = index + 1; i < cnt; i++)
            {
                UGObject* obj = Children[i];
                if (obj->GetX() != current->GetX())
                {
                    current = obj;
                    break;
                }
                k++;
            }
            for (; i < cnt; i++)
            {
                UGObject* obj = Children[i];
                if (obj->GetX() != current->GetX())
                {
                    ClearSelection();
                    AddSelection(i - k - 1, true);
                    break;
                }
            }
        }
        break;

    case 5: //down
        if (Layout == EListLayoutType::SingleColumn || Layout == EListLayoutType::FlowVertical)
        {
            index++;
            if (index < Children.Num())
            {
                ClearSelection();
                AddSelection(index, true);
            }
        }
        else if (Layout == EListLayoutType::FlowHorizontal || Layout == EListLayoutType::Pagination)
        {
            UGObject* current = Children[index];
            int32 k = 0;
            int32 cnt = Children.Num();
            int32 i;
            for (i = index + 1; i < cnt; i++)
            {
                UGObject* obj = Children[i];
                if (obj->GetY() != current->GetY())
                {
                    current = obj;
                    break;
                }
                k++;
            }
            for (; i < cnt; i++)
            {
                UGObject* obj = Children[i];
                if (obj->GetY() != current->GetY())
                {
                    ClearSelection();
                    AddSelection(i - k - 1, true);
                    break;
                }
            }
        }
        break;

    case 7: //left
        if (Layout == EListLayoutType::SingleRow || Layout == EListLayoutType::FlowHorizontal || Layout == EListLayoutType::Pagination)
        {
            index--;
            if (index >= 0)
            {
                ClearSelection();
                AddSelection(index, true);
            }
        }
        else if (Layout == EListLayoutType::FlowVertical)
        {
            UGObject* current = Children[index];
            int32 k = 0;
            int32 i;
            for (i = index - 1; i >= 0; i--)
            {
                UGObject* obj = Children[i];
                if (obj->GetX() != current->GetX())
                {
                    current = obj;
                    break;
                }
                k++;
            }
            for (; i >= 0; i--)
            {
                UGObject* obj = Children[i];
                if (obj->GetX() != current->GetX())
                {
                    ClearSelection();
                    AddSelection(i + k + 1, true);
                    break;
                }
            }
        }
        break;
    }
}

void UGList::OnClickItemHandler(UEventContext* Context)
{
    UGObject* Obj = Context->GetSender();
    if (Obj->IsA<UGButton>() && SelectionMode != EListSelectionMode::None)
        SetSelectionOnEvent(Obj, Context);

    if (ScrollPane != nullptr && bScrollItemToViewOnClick)
        ScrollPane->ScrollToView(Obj, true);

    DispatchItemEvent(Obj, Context);
}

void UGList::DispatchItemEvent(UGObject* Obj, UEventContext* Context)
{
    DispatchEvent(FUIEvents::ClickItem, FNVariant(Obj));
}

void UGList::SetSelectionOnEvent(UGObject* Obj, UEventContext* Context)
{
    bool bDontChangeLastIndex = false;
    UGButton* Button = Cast<UGButton>(Obj);
    int32 Index = ChildIndexToItemIndex(GetChildIndex(Obj));

    if (SelectionMode == EListSelectionMode::Single)
    {
        if (!Button->IsSelected())
        {
            ClearSelectionExcept(Button);
            Button->SetSelected(true);
        }
    }
    else
    {
        if (Context->GetPointerEvent().IsShiftDown())
        {
            if (!Button->IsSelected())
            {
                if (LastSelectedIndex != -1)
                {
                    int32 min = FMath::Min(LastSelectedIndex, Index);
                    int32 max = FMath::Max(LastSelectedIndex, Index);
                    max = FMath::Min(max, GetNumItems() - 1);
                    if (bVirtual)
                    {
                        for (int32 i = min; i <= max; i++)
                        {
                            FItemInfo& ii = VirtualItems[i];
                            if (ii.Obj != nullptr && ii.Obj->IsA<UGButton>())
                                Cast<UGButton>(ii.Obj)->SetSelected(true);
                            ii.bSelected = true;
                        }
                    }
                    else
                    {
                        for (int32 i = min; i <= max; i++)
                        {
                            UGButton* Child = GetChildAt(i)->As<UGButton>();
                            if (Child != nullptr && !Child->IsSelected())
                                Child->SetSelected(true);
                        }
                    }

                    bDontChangeLastIndex = true;
                }
                else
                {
                    Button->SetSelected(true);
                }
            }
        }
        else if (Context->GetPointerEvent().IsControlDown() || SelectionMode == EListSelectionMode::MultipleSingleclick)
        {
            Button->SetSelected(!Button->IsSelected());
        }
        else
        {
            if (!Button->IsSelected())
            {
                ClearSelectionExcept(Button);
                Button->SetSelected(true);
            }
            else if (Context->GetMouseButton() == EKeys::LeftMouseButton)
                ClearSelectionExcept(Button);
        }
    }

    if (!bDontChangeLastIndex)
        LastSelectedIndex = Index;

    if (Button->IsSelected())
        UpdateSelectionController(Index);
}

void UGList::ResizeToFit(int32 ItemCount, int32 InMinSize)
{
    EnsureBoundsCorrect();

    int32 curCount = GetNumItems();
    if (ItemCount > curCount)
        ItemCount = curCount;

    if (bVirtual)
    {
        int32 lineCount = FMath::CeilToInt((float)ItemCount / CurLineItemCount);
        if (Layout == EListLayoutType::SingleColumn || Layout == EListLayoutType::FlowHorizontal)
            SetViewHeight(lineCount * ItemSize.Y + FMath::Max(0, lineCount - 1) * LineGap);
        else
            SetViewWidth(lineCount * ItemSize.X + FMath::Max(0, lineCount - 1) * ColumnGap);
    }
    else if (ItemCount == 0)
    {
        if (Layout == EListLayoutType::SingleColumn || Layout == EListLayoutType::FlowHorizontal)
            SetViewHeight(InMinSize);
        else
            SetViewWidth(InMinSize);
    }
    else
    {
        int32 i = ItemCount - 1;
        UGObject* obj = nullptr;
        while (i >= 0)
        {
            obj = GetChildAt(i);
            if (!bFoldInvisibleItems || obj->IsVisible())
                break;
            i--;
        }
        if (i < 0)
        {
            if (Layout == EListLayoutType::SingleColumn || Layout == EListLayoutType::FlowHorizontal)
                SetViewHeight(InMinSize);
            else
                SetViewWidth(InMinSize);
        }
        else
        {
            float size;
            if (Layout == EListLayoutType::SingleColumn || Layout == EListLayoutType::FlowHorizontal)
            {
                size = obj->GetY() + obj->GetHeight();
                if (size < InMinSize)
                    size = InMinSize;
                SetViewHeight(size);
            }
            else
            {
                size = obj->GetX() + obj->GetWidth();
                if (size < InMinSize)
                    size = InMinSize;
                SetViewWidth(size);
            }
        }
    }
}

int32 UGList::GetFirstChildInView() const
{
    return ChildIndexToItemIndex(UGComponent::GetFirstChildInView());
}

void UGList::HandleSizeChanged()
{
    UGComponent::HandleSizeChanged();

    SetBoundsChangedFlag();
    if (bVirtual)
        SetVirtualListChangedFlag(true);
}

void UGList::HandleControllerChanged(UGController* Controller)
{
    UGComponent::HandleControllerChanged(Controller);

    if (SelectionController == Controller)
        SetSelectedIndex(Controller->GetSelectedIndex());
}

void UGList::UpdateSelectionController(int32 Index)
{
    if (SelectionController != nullptr && !SelectionController->bChanging && Index < SelectionController->GetPageCount())
    {
        UGController* Controller = SelectionController;
        SelectionController = nullptr;
        Controller->SetSelectedIndex(Index);
        SelectionController = Controller;
    }
}

void UGList::ScrollToView(int32 Index, bool bAnimation, bool bSetFirst)
{
    if (bVirtual)
    {
        if (NumItems == 0)
            return;

        CheckVirtualList();

        verifyf(Index >= 0 && Index < VirtualItems.Num(), TEXT("Invalid child index"));

        if (bLoop)
            Index = FMath::FloorToFloat(FirstIndex / NumItems) * NumItems + Index;

        FBox2D rect;
        FItemInfo& ii = VirtualItems[Index];
        if (Layout == EListLayoutType::SingleColumn || Layout == EListLayoutType::FlowHorizontal)
        {
            float pos = 0;
            for (int32 i = CurLineItemCount - 1; i < Index; i += CurLineItemCount)
                pos += VirtualItems[i].Size.Y + LineGap;
            rect.Min.Set(0, pos);
            rect.Max = rect.Min + FVector2D(ItemSize.X, ii.Size.Y);
        }
        else if (Layout == EListLayoutType::SingleRow || Layout == EListLayoutType::FlowVertical)
        {
            float pos = 0;
            for (int32 i = CurLineItemCount - 1; i < Index; i += CurLineItemCount)
                pos += VirtualItems[i].Size.X + ColumnGap;
            rect.Min.Set(pos, 0);
            rect.Max = rect.Min + FVector2D(ii.Size.X, ItemSize.Y);
        }
        else
        {
            int32 page = Index / (CurLineItemCount * CurLineItemCount2);
            rect.Min.Set(page * GetViewWidth() + (Index % CurLineItemCount) * (ii.Size.X + ColumnGap),
                (Index / CurLineItemCount) % CurLineItemCount2 * (ii.Size.Y + LineGap));
            rect.Max = rect.Min + ii.Size;
        }

        if (ScrollPane != nullptr)
            ScrollPane->ScrollToView(rect, bAnimation, bSetFirst);
        else if (Parent.IsValid() && Parent->GetScrollPane() != nullptr)
        {
            FBox2D rect2 = LocalToGlobalRect(rect);
            rect2 = Parent->GlobalToLocalRect(rect2);
            Parent->GetScrollPane()->ScrollToView(rect2, bAnimation, bSetFirst);
        }
    }
    else
    {
        UGObject* obj = GetChildAt(Index);
        if (ScrollPane != nullptr)
            ScrollPane->ScrollToView(obj, bAnimation, bSetFirst);
        else if (Parent.IsValid() && Parent->GetScrollPane() != nullptr)
            Parent->GetScrollPane()->ScrollToView(obj, bAnimation, bSetFirst);
    }
}

int32 UGList::ChildIndexToItemIndex(int32 Index) const
{
    if (!bVirtual)
        return Index;

    if (Layout == EListLayoutType::Pagination)
    {
        for (int32 i = FirstIndex; i < RealNumItems; i++)
        {
            if (VirtualItems[i].Obj != nullptr)
            {
                Index--;
                if (Index < 0)
                    return i;
            }
        }

        return Index;
    }
    else
    {
        Index += FirstIndex;
        if (bLoop && NumItems > 0)
            Index = Index % NumItems;

        return Index;
    }
}

int32 UGList::ItemIndexToChildIndex(int32 Index) const
{
    if (!bVirtual)
        return Index;

    if (Layout == EListLayoutType::Pagination)
    {
        return GetChildIndex(VirtualItems[Index].Obj);
    }
    else
    {
        if (bLoop && NumItems > 0)
        {
            int32 j = FirstIndex % NumItems;
            if (Index >= j)
                Index = Index - j;
            else
                Index = NumItems - j + Index;
        }
        else
            Index -= FirstIndex;

        return Index;
    }
}

void UGList::SetVirtual()
{
    SetVirtual(false);
}

void UGList::SetVirtualAndLoop()
{
    SetVirtual(true);
}

void UGList::SetVirtual(bool bInLoop)
{
    if (!bVirtual)
    {
        verifyf(ScrollPane != nullptr, TEXT("FairyGUI: Virtual list must be scrollable!"));

        if (bInLoop)
        {
            verifyf(Layout != EListLayoutType::FlowHorizontal && Layout != EListLayoutType::FlowVertical,
                TEXT("Loop list is not supported for FlowHorizontal or FlowVertical layout!"));

            ScrollPane->bBouncebackEffect = false;
        }

        bVirtual = true;
        bLoop = bInLoop;
        RemoveChildrenToPool();

        if (ItemSize.X == 0 || ItemSize.Y == 0)
        {
            UGObject* obj = GetFromPool();
            verifyf(obj != nullptr, TEXT("Virtual List must have a default list item resource."));
            ItemSize = obj->GetSize();
            ItemSize.X = FMath::CeilToFloat(ItemSize.X);
            ItemSize.Y = FMath::CeilToFloat(ItemSize.Y);
            ReturnToPool(obj);
        }

        if (Layout == EListLayoutType::SingleColumn || Layout == EListLayoutType::FlowHorizontal)
        {
            ScrollPane->ScrollStep = ItemSize.Y;
            if (bLoop)
                ScrollPane->LoopMode = 2;
        }
        else
        {
            ScrollPane->ScrollStep = ItemSize.X;
            if (bLoop)
                ScrollPane->LoopMode = 1;
        }

        On(FUIEvents::Scroll).AddUObject(this, &UGList::OnScrollHandler);
        SetVirtualListChangedFlag(true);
    }
}

int32 UGList::GetNumItems() const
{
    if (bVirtual)
        return NumItems;
    else
        return Children.Num();
}

void UGList::SetNumItems(int32 InNumItems)
{
    if (bVirtual)
    {
        verifyf(ItemRenderer.IsBound(), TEXT("Set itemRenderer first!"));

        NumItems = InNumItems;
        if (bLoop)
            RealNumItems = NumItems * 6;
        else
            RealNumItems = NumItems;

        int32 oldCount = VirtualItems.Num();
        if (RealNumItems > oldCount)
        {
            for (int32 i = oldCount; i < RealNumItems; i++)
            {
                FItemInfo ii;
                ii.Size = ItemSize;

                VirtualItems.Add(MoveTemp(ii));
            }
        }
        else
        {
            for (int32 i = RealNumItems; i < oldCount; i++)
                VirtualItems[i].bSelected = false;
        }

        if (VirtualListChanged != 0)
            GetApp()->CancelDelayCall(RefreshTimerHandle);

        DoRefreshVirtualList();
    }
    else
    {
        int32 cnt = Children.Num();
        if (InNumItems > cnt)
        {
            for (int32 i = cnt; i < InNumItems; i++)
            {
                if (!ItemProvider.IsBound())
                    AddItemFromPool();
                else
                    AddItemFromPool(ItemProvider.Execute(i));
            }
        }
        else
        {
            RemoveChildrenToPool(InNumItems, cnt);
        }

        if (ItemRenderer.IsBound())
        {
            for (int32 i = 0; i < InNumItems; i++)
                ItemRenderer.Execute(i, GetChildAt(i));
        }
    }
}

void UGList::RefreshVirtualList()
{
    verifyf(bVirtual, TEXT("not virtual list"));

    SetVirtualListChangedFlag(false);
}

FVector2D UGList::GetSnappingPosition(const FVector2D& InPoint)
{
    if (bVirtual)
    {
        FVector2D ret = InPoint;
        if (Layout == EListLayoutType::SingleColumn || Layout == EListLayoutType::FlowHorizontal)
        {
            int32 index = GetIndexOnPos1(ret.Y, false);
            if (index < VirtualItems.Num() && InPoint.Y - ret.Y > VirtualItems[index].Size.Y / 2 && index < RealNumItems)
                ret.Y += VirtualItems[index].Size.Y + LineGap;
        }
        else if (Layout == EListLayoutType::SingleRow || Layout == EListLayoutType::FlowVertical)
        {
            int32 index = GetIndexOnPos2(ret.X, false);
            if (index < VirtualItems.Num() && InPoint.X - ret.X > VirtualItems[index].Size.X / 2 && index < RealNumItems)
                ret.X += VirtualItems[index].Size.X + ColumnGap;
        }
        else
        {
            int32 index = GetIndexOnPos3(ret.X, false);
            if (index < VirtualItems.Num() && InPoint.X - ret.X > VirtualItems[index].Size.X / 2 && index < RealNumItems)
                ret.X += VirtualItems[index].Size.X + ColumnGap;
        }

        return ret;
    }
    else
        return UGComponent::GetSnappingPosition(InPoint);
}

void UGList::CheckVirtualList()
{
    if (VirtualListChanged != 0)
    {
        DoRefreshVirtualList();
        GetApp()->CancelDelayCall(RefreshTimerHandle);
    }
}

void UGList::SetVirtualListChangedFlag(bool bLayoutChanged)
{
    if (bLayoutChanged)
        VirtualListChanged = 2;
    else if (VirtualListChanged == 0)
        VirtualListChanged = 1;

    GetApp()->DelayCall(RefreshTimerHandle, this, &UGList::DoRefreshVirtualList);
}

void UGList::DoRefreshVirtualList()
{
    bool bLayoutChanged = VirtualListChanged == 2;
    VirtualListChanged = 0;
    bEventLocked = true;

    if (bLayoutChanged)
    {
        if (Layout == EListLayoutType::SingleColumn || Layout == EListLayoutType::SingleRow)
            CurLineItemCount = 1;
        else if (Layout == EListLayoutType::FlowHorizontal)
        {
            if (ColumnCount > 0)
                CurLineItemCount = ColumnCount;
            else
            {
                CurLineItemCount = FMath::FloorToInt((ScrollPane->GetViewSize().X + ColumnGap) / (ItemSize.X + ColumnGap));
                if (CurLineItemCount <= 0)
                    CurLineItemCount = 1;
            }
        }
        else if (Layout == EListLayoutType::FlowVertical)
        {
            if (LineCount > 0)
                CurLineItemCount = LineCount;
            else
            {
                CurLineItemCount = FMath::FloorToInt((ScrollPane->GetViewSize().Y + LineGap) / (ItemSize.Y + LineGap));
                if (CurLineItemCount <= 0)
                    CurLineItemCount = 1;
            }
        }
        else //Pagination
        {
            if (ColumnCount > 0)
                CurLineItemCount = ColumnCount;
            else
            {
                CurLineItemCount = FMath::FloorToInt((ScrollPane->GetViewSize().X + ColumnGap) / (ItemSize.X + ColumnGap));
                if (CurLineItemCount <= 0)
                    CurLineItemCount = 1;
            }

            if (LineCount > 0)
                CurLineItemCount2 = LineCount;
            else
            {
                CurLineItemCount2 = FMath::FloorToInt((ScrollPane->GetViewSize().Y + LineGap) / (ItemSize.Y + LineGap));
                if (CurLineItemCount2 <= 0)
                    CurLineItemCount2 = 1;
            }
        }
    }
    float ch = 0, cw = 0;
    if (RealNumItems > 0)
    {
        int32 len = FMath::FloorToInt((float)RealNumItems / CurLineItemCount) * CurLineItemCount;
        int32 len2 = FMath::Min(CurLineItemCount, RealNumItems);
        if (Layout == EListLayoutType::SingleColumn || Layout == EListLayoutType::FlowHorizontal)
        {
            for (int32 i = 0; i < len; i += CurLineItemCount)
                ch += VirtualItems[i].Size.Y + LineGap;
            if (ch > 0)
                ch -= LineGap;

            if (bAutoResizeItem)
                cw = ScrollPane->GetViewSize().X;
            else
            {
                for (int32 i = 0; i < len2; i++)
                    cw += VirtualItems[i].Size.X + ColumnGap;
                if (cw > 0)
                    cw -= ColumnGap;
            }
        }
        else if (Layout == EListLayoutType::SingleRow || Layout == EListLayoutType::FlowVertical)
        {
            for (int32 i = 0; i < len; i += CurLineItemCount)
                cw += VirtualItems[i].Size.X + ColumnGap;
            if (cw > 0)
                cw -= ColumnGap;

            if (bAutoResizeItem)
                ch = ScrollPane->GetViewSize().Y;
            else
            {
                for (int32 i = 0; i < len2; i++)
                    ch += VirtualItems[i].Size.Y + LineGap;
                if (ch > 0)
                    ch -= LineGap;
            }
        }
        else
        {
            int32 pageCount = FMath::CeilToInt((float)len / (CurLineItemCount * CurLineItemCount2));
            cw = pageCount * GetViewWidth();
            ch = GetViewHeight();
        }
    }

    HandleAlign(cw, ch);
    ScrollPane->SetContentSize(FVector2D(cw, ch));

    bEventLocked = false;

    HandleScroll(true);
}

void UGList::OnScrollHandler(UEventContext* Context)
{
    HandleScroll(false);
}

int32 UGList::GetIndexOnPos1(float& pos, bool forceUpdate)
{
    if (RealNumItems < CurLineItemCount)
    {
        pos = 0;
        return 0;
    }

    if (NumChildren() > 0 && !forceUpdate)
    {
        float pos2 = GetChildAt(0)->GetY();
        if (pos2 + (LineGap > 0 ? 0 : -LineGap) > pos)
        {
            for (int32 i = FirstIndex - CurLineItemCount; i >= 0; i -= CurLineItemCount)
            {
                pos2 -= (VirtualItems[i].Size.Y + LineGap);
                if (pos2 <= pos)
                {
                    pos = pos2;
                    return i;
                }
            }

            pos = 0;
            return 0;
        }
        else
        {
            float testGap = LineGap > 0 ? LineGap : 0;
            for (int32 i = FirstIndex; i < RealNumItems; i += CurLineItemCount)
            {
                float pos3 = pos2 + VirtualItems[i].Size.Y;
                if (pos3 + testGap > pos)
                {
                    pos = pos2;
                    return i;
                }
                pos2 = pos3 + LineGap;
            }

            pos = pos2;
            return RealNumItems - CurLineItemCount;
        }
    }
    else
    {
        float pos2 = 0;
        float testGap = LineGap > 0 ? LineGap : 0;
        for (int32 i = 0; i < RealNumItems; i += CurLineItemCount)
        {
            float pos3 = pos2 + VirtualItems[i].Size.Y;
            if (pos3 + testGap > pos)
            {
                pos = pos2;
                return i;
            }
            pos2 = pos3 + LineGap;
        }

        pos = pos2;
        return RealNumItems - CurLineItemCount;
    }
}

int32 UGList::GetIndexOnPos2(float& pos, bool forceUpdate)
{
    if (RealNumItems < CurLineItemCount)
    {
        pos = 0;
        return 0;
    }

    if (NumChildren() > 0 && !forceUpdate)
    {
        float pos2 = GetChildAt(0)->GetX();
        if (pos2 + (ColumnGap > 0 ? 0 : -ColumnGap) > pos)
        {
            for (int32 i = FirstIndex - CurLineItemCount; i >= 0; i -= CurLineItemCount)
            {
                pos2 -= (VirtualItems[i].Size.X + ColumnGap);
                if (pos2 <= pos)
                {
                    pos = pos2;
                    return i;
                }
            }

            pos = 0;
            return 0;
        }
        else
        {
            float testGap = ColumnGap > 0 ? ColumnGap : 0;
            for (int32 i = FirstIndex; i < RealNumItems; i += CurLineItemCount)
            {
                float pos3 = pos2 + VirtualItems[i].Size.X;
                if (pos3 + testGap > pos)
                {
                    pos = pos2;
                    return i;
                }
                pos2 = pos3 + ColumnGap;
            }

            pos = pos2;
            return RealNumItems - CurLineItemCount;
        }
    }
    else
    {
        float pos2 = 0;
        float testGap = ColumnGap > 0 ? ColumnGap : 0;
        for (int32 i = 0; i < RealNumItems; i += CurLineItemCount)
        {
            float pos3 = pos2 + VirtualItems[i].Size.X;
            if (pos3 + testGap > pos)
            {
                pos = pos2;
                return i;
            }
            pos2 = pos3 + ColumnGap;
        }

        pos = pos2;
        return RealNumItems - CurLineItemCount;
    }
}

int32 UGList::GetIndexOnPos3(float& pos, bool forceUpdate)
{
    if (RealNumItems < CurLineItemCount)
    {
        pos = 0;
        return 0;
    }

    float viewWidth = GetViewWidth();
    int32 page = FMath::FloorToInt(pos / viewWidth);
    int32 startIndex = page * (CurLineItemCount * CurLineItemCount2);
    float pos2 = page * viewWidth;
    float testGap = ColumnGap > 0 ? ColumnGap : 0;
    for (int32 i = 0; i < CurLineItemCount; i++)
    {
        float pos3 = pos2 + VirtualItems[startIndex + i].Size.X;
        if (pos3 + testGap > pos)
        {
            pos = pos2;
            return startIndex + i;
        }
        pos2 = pos3 + ColumnGap;
    }

    pos = pos2;
    return startIndex + CurLineItemCount - 1;
}

void UGList::HandleScroll(bool forceUpdate)
{
    if (bEventLocked)
        return;

    if (Layout == EListLayoutType::SingleColumn || Layout == EListLayoutType::FlowHorizontal)
    {
        int32 enterCounter = 0;
        while (HandleScroll1(forceUpdate))
        {
            enterCounter++;
            forceUpdate = false;
            if (enterCounter > 20)
            {
                UE_LOG(LogFairyGUI, Warning, TEXT("list will never be filled as the item renderer function always returns a different size."));
                break;
            }
        }
        HandleArchOrder1();
    }
    else if (Layout == EListLayoutType::SingleRow || Layout == EListLayoutType::FlowVertical)
    {
        int32 enterCounter = 0;
        while (HandleScroll2(forceUpdate))
        {
            enterCounter++;
            forceUpdate = false;
            if (enterCounter > 20)
            {
                UE_LOG(LogFairyGUI, Warning, TEXT("list will never be filled as the item renderer function always returns a different size."));
                break;
            }
        }
        HandleArchOrder2();
    }
    else
    {
        HandleScroll3(forceUpdate);
    }

    bBoundsChanged = false;
}

bool UGList::HandleScroll1(bool forceUpdate)
{
    float pos = ScrollPane->GetScrollingPosY();
    float max = pos + ScrollPane->GetViewSize().Y;
    bool end = max == ScrollPane->GetContentSize().Y;

    int32 newFirstIndex = GetIndexOnPos1(pos, forceUpdate);
    if (newFirstIndex == FirstIndex && !forceUpdate)
        return false;

    int32 oldFirstIndex = FirstIndex;
    FirstIndex = newFirstIndex;
    int32 curIndex = newFirstIndex;
    bool forward = oldFirstIndex > newFirstIndex;
    int32 childCount = NumChildren();
    int32 lastIndex = oldFirstIndex + childCount - 1;
    int32 reuseIndex = forward ? lastIndex : oldFirstIndex;
    float curX = 0, curY = pos;
    bool needRender;
    float deltaSize = 0;
    float firstItemDeltaSize = 0;
    FString url = DefaultItem;
    int32 partSize = (int32)((ScrollPane->GetViewSize().X - ColumnGap * (CurLineItemCount - 1)) / CurLineItemCount);

    ItemInfoVer++;
    while (curIndex < RealNumItems && (end || curY < max))
    {
        FItemInfo& ii = VirtualItems[curIndex];

        if (ii.Obj == nullptr || forceUpdate)
        {
            if (ItemProvider.IsBound())
            {
                url = ItemProvider.Execute(curIndex % NumItems);
                if (url.Len() == 0)
                    url = DefaultItem;
                url = UUIPackage::NormalizeURL(url);
            }

            if (ii.Obj != nullptr && ii.Obj->GetResourceURL().Compare(url) != 0)
            {
                if (Cast<UGButton>(ii.Obj))
                    ii.bSelected = ((UGButton*)ii.Obj)->IsSelected();
                RemoveChildToPool(ii.Obj);
                ii.Obj = nullptr;
            }
        }

        if (ii.Obj == nullptr)
        {
            if (forward)
            {
                for (int32 j = reuseIndex; j >= oldFirstIndex; j--)
                {
                    FItemInfo& ii2 = VirtualItems[j];
                    if (ii2.Obj != nullptr && ii2.UpdateFlag != ItemInfoVer && ii2.Obj->GetResourceURL().Compare(url) == 0)
                    {
                        if (Cast<UGButton>(ii2.Obj))
                            ii2.bSelected = ((UGButton*)ii2.Obj)->IsSelected();
                        ii.Obj = ii2.Obj;
                        ii2.Obj = nullptr;
                        if (j == reuseIndex)
                            reuseIndex--;
                        break;
                    }
                }
            }
            else
            {
                for (int32 j = reuseIndex; j <= lastIndex; j++)
                {
                    FItemInfo& ii2 = VirtualItems[j];
                    if (ii2.Obj != nullptr && ii2.UpdateFlag != ItemInfoVer && ii2.Obj->GetResourceURL().Compare(url) == 0)
                    {
                        if (Cast<UGButton>(ii2.Obj))
                            ii2.bSelected = ((UGButton*)ii2.Obj)->IsSelected();
                        ii.Obj = ii2.Obj;
                        ii2.Obj = nullptr;
                        if (j == reuseIndex)
                            reuseIndex++;
                        break;
                    }
                }
            }

            if (ii.Obj != nullptr)
            {
                SetChildIndex(ii.Obj, forward ? curIndex - newFirstIndex : NumChildren());
            }
            else
            {
                ii.Obj = Pool->GetObject(url, this);
                if (forward)
                    AddChildAt(ii.Obj, curIndex - newFirstIndex);
                else
                    AddChild(ii.Obj);
            }
            if (Cast<UGButton>(ii.Obj))
                ((UGButton*)ii.Obj)->SetSelected(ii.bSelected);

            needRender = true;
        }
        else
            needRender = forceUpdate;

        if (needRender)
        {
            if (bAutoResizeItem && (Layout == EListLayoutType::SingleColumn || ColumnCount > 0))
                ii.Obj->SetSize(FVector2D(partSize, ii.Obj->GetHeight()), true);

            ItemRenderer.ExecuteIfBound(curIndex % NumItems, ii.Obj);
            if (curIndex % CurLineItemCount == 0)
            {
                deltaSize += FMath::CeilToFloat(ii.Obj->GetHeight()) - ii.Size.Y;
                if (curIndex == newFirstIndex && oldFirstIndex > newFirstIndex)
                {
                    firstItemDeltaSize = FMath::CeilToFloat(ii.Obj->GetHeight()) - ii.Size.Y;
                }
            }
            ii.Size.X = FMath::CeilToFloat(ii.Obj->GetWidth());
            ii.Size.Y = FMath::CeilToFloat(ii.Obj->GetHeight());
        }

        ii.UpdateFlag = ItemInfoVer;
        ii.Obj->SetPosition(FVector2D(curX, curY));
        if (curIndex == newFirstIndex)
            max += ii.Size.Y;

        curX += ii.Size.X + ColumnGap;

        if (curIndex % CurLineItemCount == CurLineItemCount - 1)
        {
            curX = 0;
            curY += ii.Size.Y + LineGap;
        }
        curIndex++;
    }

    for (int32 i = 0; i < childCount; i++)
    {
        FItemInfo& ii = VirtualItems[oldFirstIndex + i];
        if (ii.UpdateFlag != ItemInfoVer && ii.Obj != nullptr)
        {
            if (Cast<UGButton>(ii.Obj))
                ii.bSelected = ((UGButton*)ii.Obj)->IsSelected();
            RemoveChildToPool(ii.Obj);
            ii.Obj = nullptr;
        }
    }

    childCount = Children.Num();
    for (int32 i = 0; i < childCount; i++)
    {
        UGObject* obj = VirtualItems[newFirstIndex + i].Obj;
        if (Children[i] != obj)
            SetChildIndex(obj, i);
    }

    if (deltaSize != 0 || firstItemDeltaSize != 0)
        ScrollPane->ChangeContentSizeOnScrolling(0, deltaSize, 0, firstItemDeltaSize);

    if (curIndex > 0 && NumChildren() > 0 && Container->GetPosition().Y <= 0 && GetChildAt(0)->GetY() > -Container->GetPosition().Y)
        return true;
    else
        return false;
}

bool UGList::HandleScroll2(bool forceUpdate)
{
    float pos = ScrollPane->GetScrollingPosX();
    float max = pos + ScrollPane->GetViewSize().X;
    bool end = pos == ScrollPane->GetContentSize().X;

    int32 newFirstIndex = GetIndexOnPos2(pos, forceUpdate);
    if (newFirstIndex == FirstIndex && !forceUpdate)
        return false;

    int32 oldFirstIndex = FirstIndex;
    FirstIndex = newFirstIndex;
    int32 curIndex = newFirstIndex;
    bool forward = oldFirstIndex > newFirstIndex;
    int32 childCount = NumChildren();
    int32 lastIndex = oldFirstIndex + childCount - 1;
    int32 reuseIndex = forward ? lastIndex : oldFirstIndex;
    float curX = pos, curY = 0;
    bool needRender;
    float deltaSize = 0;
    float firstItemDeltaSize = 0;
    FString url = DefaultItem;
    int32 partSize = (int32)((ScrollPane->GetViewSize().Y - LineGap * (CurLineItemCount - 1)) / CurLineItemCount);

    ItemInfoVer++;
    while (curIndex < RealNumItems && (end || curX < max))
    {
        FItemInfo& ii = VirtualItems[curIndex];

        if (ii.Obj == nullptr || forceUpdate)
        {
            if (ItemProvider.IsBound())
            {
                url = ItemProvider.Execute(curIndex % NumItems);
                if (url.Len() == 0)
                    url = DefaultItem;
                url = UUIPackage::NormalizeURL(url);
            }

            if (ii.Obj != nullptr && ii.Obj->GetResourceURL().Compare(url) != 0)
            {
                if (Cast<UGButton>(ii.Obj))
                    ii.bSelected = ((UGButton*)ii.Obj)->IsSelected();
                RemoveChildToPool(ii.Obj);
                ii.Obj = nullptr;
            }
        }

        if (ii.Obj == nullptr)
        {
            if (forward)
            {
                for (int32 j = reuseIndex; j >= oldFirstIndex; j--)
                {
                    FItemInfo& ii2 = VirtualItems[j];
                    if (ii2.Obj != nullptr && ii2.UpdateFlag != ItemInfoVer && ii2.Obj->GetResourceURL().Compare(url) == 0)
                    {
                        if (Cast<UGButton>(ii2.Obj))
                            ii2.bSelected = ((UGButton*)ii2.Obj)->IsSelected();
                        ii.Obj = ii2.Obj;
                        ii2.Obj = nullptr;
                        if (j == reuseIndex)
                            reuseIndex--;
                        break;
                    }
                }
            }
            else
            {
                for (int32 j = reuseIndex; j <= lastIndex; j++)
                {
                    FItemInfo& ii2 = VirtualItems[j];
                    if (ii2.Obj != nullptr && ii2.UpdateFlag != ItemInfoVer && ii2.Obj->GetResourceURL().Compare(url) == 0)
                    {
                        if (Cast<UGButton>(ii2.Obj))
                            ii2.bSelected = ((UGButton*)ii2.Obj)->IsSelected();
                        ii.Obj = ii2.Obj;
                        ii2.Obj = nullptr;
                        if (j == reuseIndex)
                            reuseIndex++;
                        break;
                    }
                }
            }

            if (ii.Obj != nullptr)
            {
                SetChildIndex(ii.Obj, forward ? curIndex - newFirstIndex : NumChildren());
            }
            else
            {
                ii.Obj = Pool->GetObject(url, this);
                if (forward)
                    AddChildAt(ii.Obj, curIndex - newFirstIndex);
                else
                    AddChild(ii.Obj);
            }
            if (Cast<UGButton>(ii.Obj))
                ((UGButton*)ii.Obj)->SetSelected(ii.bSelected);

            needRender = true;
        }
        else
            needRender = forceUpdate;

        if (needRender)
        {
            if (bAutoResizeItem && (Layout == EListLayoutType::SingleRow || LineCount > 0))
                ii.Obj->SetSize(FVector2D(ii.Obj->GetWidth(), partSize), true);

            ItemRenderer.ExecuteIfBound(curIndex % NumItems, ii.Obj);
            if (curIndex % CurLineItemCount == 0)
            {
                deltaSize += FMath::CeilToFloat(ii.Obj->GetWidth()) - ii.Size.X;
                if (curIndex == newFirstIndex && oldFirstIndex > newFirstIndex)
                {
                    firstItemDeltaSize = FMath::CeilToFloat(ii.Obj->GetWidth()) - ii.Size.X;
                }
            }
            ii.Size.X = FMath::CeilToFloat(ii.Obj->GetWidth());
            ii.Size.Y = FMath::CeilToFloat(ii.Obj->GetHeight());
        }

        ii.UpdateFlag = ItemInfoVer;
        ii.Obj->SetPosition(FVector2D(curX, curY));
        if (curIndex == newFirstIndex)
            max += ii.Size.X;

        curY += ii.Size.Y + LineGap;

        if (curIndex % CurLineItemCount == CurLineItemCount - 1)
        {
            curY = 0;
            curX += ii.Size.X + ColumnGap;
        }
        curIndex++;
    }

    for (int32 i = 0; i < childCount; i++)
    {
        FItemInfo& ii = VirtualItems[oldFirstIndex + i];
        if (ii.UpdateFlag != ItemInfoVer && ii.Obj != nullptr)
        {
            if (Cast<UGButton>(ii.Obj))
                ii.bSelected = ((UGButton*)ii.Obj)->IsSelected();
            RemoveChildToPool(ii.Obj);
            ii.Obj = nullptr;
        }
    }

    childCount = Children.Num();
    for (int32 i = 0; i < childCount; i++)
    {
        UGObject* obj = VirtualItems[newFirstIndex + i].Obj;
        if (Children[i] != obj)
            SetChildIndex(obj, i);
    }

    if (deltaSize != 0 || firstItemDeltaSize != 0)
        ScrollPane->ChangeContentSizeOnScrolling(deltaSize, 0, firstItemDeltaSize, 0);

    if (curIndex > 0 && NumChildren() > 0 && Container->GetPosition().X <= 0 && GetChildAt(0)->GetX() > -Container->GetPosition().X)
        return true;
    else
        return false;
}

void UGList::HandleScroll3(bool forceUpdate)
{
    float pos = ScrollPane->GetScrollingPosX();

    int32 newFirstIndex = GetIndexOnPos3(pos, forceUpdate);
    if (newFirstIndex == FirstIndex && !forceUpdate)
        return;

    int32 oldFirstIndex = FirstIndex;
    FirstIndex = newFirstIndex;

    int32 reuseIndex = oldFirstIndex;
    int32 virtualItemCount = VirtualItems.Num();
    int32 pageSize = CurLineItemCount * CurLineItemCount2;
    int32 startCol = newFirstIndex % CurLineItemCount;
    float viewWidth = GetViewWidth();
    int32 page = (int32)(newFirstIndex / pageSize);
    int32 startIndex = page * pageSize;
    int32 lastIndex = startIndex + pageSize * 2;
    bool needRender;
    FString url = DefaultItem;
    int32 partWidth = (int32)((ScrollPane->GetViewSize().X - ColumnGap * (CurLineItemCount - 1)) / CurLineItemCount);
    int32 partHeight = (int32)((ScrollPane->GetViewSize().Y - LineGap * (CurLineItemCount2 - 1)) / CurLineItemCount2);
    ItemInfoVer++;

    for (int32 i = startIndex; i < lastIndex; i++)
    {
        if (i >= RealNumItems)
            continue;

        int32 col = i % CurLineItemCount;
        if (i - startIndex < pageSize)
        {
            if (col < startCol)
                continue;
        }
        else
        {
            if (col > startCol)
                continue;
        }

        FItemInfo& ii = VirtualItems[i];
        ii.UpdateFlag = ItemInfoVer;
    }

    UGObject* lastObj = nullptr;
    int32 insertIndex = 0;
    for (int32 i = startIndex; i < lastIndex; i++)
    {
        if (i >= RealNumItems)
            continue;

        FItemInfo& ii = VirtualItems[i];
        if (ii.UpdateFlag != ItemInfoVer)
            continue;

        if (ii.Obj == nullptr)
        {
            reuseIndex = reuseIndex < 0 ? 0 : reuseIndex;
            while (reuseIndex < virtualItemCount)
            {
                FItemInfo& ii2 = VirtualItems[reuseIndex];
                if (ii2.Obj != nullptr && ii2.UpdateFlag != ItemInfoVer)
                {
                    if (Cast<UGButton>(ii2.Obj))
                        ii2.bSelected = ((UGButton*)ii2.Obj)->IsSelected();
                    ii.Obj = ii2.Obj;
                    ii2.Obj = nullptr;
                    break;
                }
                reuseIndex++;
            }

            if (insertIndex == -1)
                insertIndex = GetChildIndex(lastObj) + 1;

            if (ii.Obj == nullptr)
            {
                if (ItemProvider.IsBound())
                {
                    url = ItemProvider.Execute(i % NumItems);
                    if (url.Len() == 0)
                        url = DefaultItem;
                    url = UUIPackage::NormalizeURL(url);
                }

                ii.Obj = Pool->GetObject(url, this);
                AddChildAt(ii.Obj, insertIndex);
            }
            else
            {
                insertIndex = SetChildIndexBefore(ii.Obj, insertIndex);
            }
            insertIndex++;

            if (Cast<UGButton>(ii.Obj))
                ((UGButton*)ii.Obj)->SetSelected(ii.bSelected);

            needRender = true;
        }
        else
        {
            needRender = forceUpdate;
            insertIndex = -1;
            lastObj = ii.Obj;
        }

        if (needRender)
        {
            if (bAutoResizeItem)
            {
                if (CurLineItemCount == ColumnCount && CurLineItemCount2 == LineCount)
                    ii.Obj->SetSize(FVector2D(partWidth, partHeight), true);
                else if (CurLineItemCount == ColumnCount)
                    ii.Obj->SetSize(FVector2D(partWidth, ii.Obj->GetHeight()), true);
                else if (CurLineItemCount2 == LineCount)
                    ii.Obj->SetSize(FVector2D(ii.Obj->GetWidth(), partHeight), true);
            }

            ItemRenderer.ExecuteIfBound(i % NumItems, ii.Obj);
            ii.Size.X = FMath::CeilToFloat(ii.Obj->GetWidth());
            ii.Size.Y = FMath::CeilToFloat(ii.Obj->GetHeight());
        }
    }

    float borderX = (startIndex / pageSize) * viewWidth;
    float xx = borderX;
    float yy = 0;
    float lineHeight = 0;
    for (int32 i = startIndex; i < lastIndex; i++)
    {
        if (i >= RealNumItems)
            continue;

        FItemInfo& ii = VirtualItems[i];
        if (ii.UpdateFlag == ItemInfoVer)
            ii.Obj->SetPosition(FVector2D(xx, yy));

        if (ii.Size.Y > lineHeight)
            lineHeight = ii.Size.Y;
        if (i % CurLineItemCount == CurLineItemCount - 1)
        {
            xx = borderX;
            yy += lineHeight + LineGap;
            lineHeight = 0;

            if (i == startIndex + pageSize - 1)
            {
                borderX += viewWidth;
                xx = borderX;
                yy = 0;
            }
        }
        else
            xx += ii.Size.X + ColumnGap;
    }

    for (int32 i = reuseIndex; i < virtualItemCount; i++)
    {
        FItemInfo& ii = VirtualItems[i];
        if (ii.UpdateFlag != ItemInfoVer && ii.Obj != nullptr)
        {
            if (Cast<UGButton>(ii.Obj))
                ii.bSelected = ((UGButton*)ii.Obj)->IsSelected();
            RemoveChildToPool(ii.Obj);
            ii.Obj = nullptr;
        }
    }
}

void UGList::HandleArchOrder1()
{
    if (ChildrenRenderOrder == EChildrenRenderOrder::Arch)
    {
        float mid = ScrollPane->GetPosY() + GetViewHeight() / 2;
        float minDist = FLT_MAX, dist;
        int32 apexIndex = 0;
        int32 cnt = NumChildren();
        for (int32 i = 0; i < cnt; i++)
        {
            UGObject* obj = GetChildAt(i);
            if (!bFoldInvisibleItems || obj->IsVisible())
            {
                dist = FMath::Abs(mid - obj->GetY() - obj->GetHeight() / 2);
                if (dist < minDist)
                {
                    minDist = dist;
                    apexIndex = i;
                }
            }
        }
        SetApexIndex(apexIndex);
    }
}

void UGList::HandleArchOrder2()
{
    if (ChildrenRenderOrder == EChildrenRenderOrder::Arch)
    {
        float mid = ScrollPane->GetPosX() + GetViewWidth() / 2;
        float minDist = FLT_MAX, dist;
        int32 apexIndex = 0;
        int32 cnt = NumChildren();
        for (int32 i = 0; i < cnt; i++)
        {
            UGObject* obj = GetChildAt(i);
            if (!bFoldInvisibleItems || obj->IsVisible())
            {
                dist = FMath::Abs(mid - obj->GetX() - obj->GetWidth() / 2);
                if (dist < minDist)
                {
                    minDist = dist;
                    apexIndex = i;
                }
            }
        }
        SetApexIndex(apexIndex);
    }
}

void UGList::HandleAlign(float contentWidth, float contentHeight)
{
    FVector2D newOffset(0, 0);

    float viewHeight = GetViewHeight();
    float viewWidth = GetViewWidth();
    if (contentHeight < viewHeight)
    {
        if (VerticalAlign == EVerticalAlignType::Middle)
            newOffset.Y = (int32)((viewHeight - contentHeight) / 2);
        else if (VerticalAlign == EVerticalAlignType::Bottom)
            newOffset.Y = viewHeight - contentHeight;
    }

    if (contentWidth < viewWidth)
    {
        if (Align == EAlignType::Center)
            newOffset.X = (int32)((viewWidth - contentWidth) / 2);
        else if (Align == EAlignType::Right)
            newOffset.X = viewWidth - contentWidth;
    }

    if (newOffset != AlignOffset)
    {
        AlignOffset = newOffset;
        if (ScrollPane != nullptr)
            ScrollPane->AdjustMaskContainer();
        else
            Container->SetPosition(FVector2D(Margin.Left + AlignOffset.X, Margin.Top + AlignOffset.Y));
    }
}

void UGList::UpdateBounds()
{
    if (bVirtual)
        return;

    int32 cnt = Children.Num();
    int32 i;
    int32 j = 0;
    UGObject* child;
    float curX = 0;
    float curY = 0;
    float cw, ch;
    float maxWidth = 0;
    float maxHeight = 0;
    float viewWidth = GetViewWidth();
    float viewHeight = GetViewHeight();

    if (Layout == EListLayoutType::SingleColumn)
    {
        for (i = 0; i < cnt; i++)
        {
            child = GetChildAt(i);
            if (bFoldInvisibleItems && !child->IsVisible())
                continue;

            if (curY != 0)
                curY += LineGap;
            child->SetY(curY);
            if (bAutoResizeItem)
                child->SetSize(FVector2D(viewWidth, child->GetHeight()), true);
            curY += FMath::CeilToFloat(child->GetHeight());
            if (child->GetWidth() > maxWidth)
                maxWidth = child->GetWidth();
        }
        ch = curY;
        if (ch <= viewHeight && bAutoResizeItem && ScrollPane != nullptr && ScrollPane->bDisplayInDemand && ScrollPane->VtScrollBar != nullptr)
        {
            viewWidth += ScrollPane->VtScrollBar->GetWidth();
            for (i = 0; i < cnt; i++)
            {
                child = GetChildAt(i);
                if (bFoldInvisibleItems && !child->IsVisible())
                    continue;

                child->SetSize(FVector2D(viewWidth, child->GetHeight()), true);
                if (child->GetWidth() > maxWidth)
                    maxWidth = child->GetWidth();
            }
        }
        cw = FMath::CeilToFloat(maxWidth);
    }
    else if (Layout == EListLayoutType::SingleRow)
    {
        for (i = 0; i < cnt; i++)
        {
            child = GetChildAt(i);
            if (bFoldInvisibleItems && !child->IsVisible())
                continue;

            if (curX != 0)
                curX += ColumnGap;
            child->SetX(curX);
            if (bAutoResizeItem)
                child->SetSize(FVector2D(child->GetWidth(), viewHeight), true);
            curX += FMath::CeilToFloat(child->GetWidth());
            if (child->GetHeight() > maxHeight)
                maxHeight = child->GetHeight();
        }
        cw = curX;
        if (cw <= viewWidth && bAutoResizeItem && ScrollPane != nullptr && ScrollPane->bDisplayInDemand && ScrollPane->HzScrollBar != nullptr)
        {
            viewHeight += ScrollPane->HzScrollBar->GetHeight();
            for (i = 0; i < cnt; i++)
            {
                child = GetChildAt(i);
                if (bFoldInvisibleItems && !child->IsVisible())
                    continue;

                child->SetSize(FVector2D(child->GetWidth(), viewHeight), true);
                if (child->GetHeight() > maxHeight)
                    maxHeight = child->GetHeight();
            }
        }
        ch = FMath::CeilToFloat(maxHeight);
    }
    else if (Layout == EListLayoutType::FlowHorizontal)
    {
        if (bAutoResizeItem && ColumnCount > 0)
        {
            float lineSize = 0;
            int32 lineStart = 0;
            float ratio;

            for (i = 0; i < cnt; i++)
            {
                child = GetChildAt(i);
                if (bFoldInvisibleItems && !child->IsVisible())
                    continue;

                lineSize += child->SourceSize.X;
                j++;
                if (j == ColumnCount || i == cnt - 1)
                {
                    ratio = (viewWidth - lineSize - (j - 1) * ColumnGap) / lineSize;
                    curX = 0;
                    for (j = lineStart; j <= i; j++)
                    {
                        child = GetChildAt(j);
                        if (bFoldInvisibleItems && !child->IsVisible())
                            continue;

                        child->SetPosition(FVector2D(curX, curY));

                        if (j < i)
                        {
                            child->SetSize(FVector2D(child->SourceSize.X + round(child->SourceSize.X * ratio), child->GetHeight()), true);
                            curX += FMath::CeilToFloat(child->GetWidth()) + ColumnGap;
                        }
                        else
                        {
                            child->SetSize(FVector2D(viewWidth - curX, child->GetHeight()), true);
                        }
                        if (child->GetHeight() > maxHeight)
                            maxHeight = child->GetHeight();
                    }
                    curY += FMath::CeilToFloat(maxHeight) + LineGap;
                    maxHeight = 0;
                    j = 0;
                    lineStart = i + 1;
                    lineSize = 0;
                }
            }
            ch = curY + FMath::CeilToFloat(maxHeight);
            cw = viewWidth;
        }
        else
        {
            for (i = 0; i < cnt; i++)
            {
                child = GetChildAt(i);
                if (bFoldInvisibleItems && !child->IsVisible())
                    continue;

                if (curX != 0)
                    curX += ColumnGap;

                if ((ColumnCount != 0 && j >= ColumnCount) || (ColumnCount == 0 && curX + child->GetWidth() > viewWidth && maxHeight != 0))
                {
                    curX = 0;
                    curY += FMath::CeilToFloat(maxHeight) + LineGap;
                    maxHeight = 0;
                    j = 0;
                }
                child->SetPosition(FVector2D(curX, curY));
                curX += FMath::CeilToFloat(child->GetWidth());
                if (curX > maxWidth)
                    maxWidth = curX;
                if (child->GetHeight() > maxHeight)
                    maxHeight = child->GetHeight();
                j++;
            }
            ch = curY + FMath::CeilToFloat(maxHeight);
            cw = FMath::CeilToFloat(maxWidth);
        }
    }
    else if (Layout == EListLayoutType::FlowVertical)
    {
        if (bAutoResizeItem && LineCount > 0)
        {
            float lineSize = 0;
            int32 lineStart = 0;
            float ratio;

            for (i = 0; i < cnt; i++)
            {
                child = GetChildAt(i);
                if (bFoldInvisibleItems && !child->IsVisible())
                    continue;

                lineSize += child->SourceSize.Y;
                j++;
                if (j == LineCount || i == cnt - 1)
                {
                    ratio = (viewHeight - lineSize - (j - 1) * LineGap) / lineSize;
                    curY = 0;
                    for (j = lineStart; j <= i; j++)
                    {
                        child = GetChildAt(j);
                        if (bFoldInvisibleItems && !child->IsVisible())
                            continue;

                        child->SetPosition(FVector2D(curX, curY));

                        if (j < i)
                        {
                            child->SetSize(FVector2D(child->GetWidth(), child->SourceSize.Y + FMath::RoundToFloat(child->SourceSize.Y * ratio)), true);
                            curY += FMath::CeilToFloat(child->GetHeight()) + LineGap;
                        }
                        else
                        {
                            child->SetSize(FVector2D(child->GetWidth(), viewHeight - curY), true);
                        }
                        if (child->GetWidth() > maxWidth)
                            maxWidth = child->GetWidth();
                    }
                    curX += FMath::CeilToFloat(maxWidth) + ColumnGap;
                    maxWidth = 0;
                    j = 0;
                    lineStart = i + 1;
                    lineSize = 0;
                }
            }
            cw = curX + FMath::CeilToFloat(maxWidth);
            ch = viewHeight;
        }
        else
        {
            for (i = 0; i < cnt; i++)
            {
                child = GetChildAt(i);
                if (bFoldInvisibleItems && !child->IsVisible())
                    continue;

                if (curY != 0)
                    curY += LineGap;

                if ((LineCount != 0 && j >= LineCount) || (LineCount == 0 && curY + child->GetHeight() > viewHeight && maxWidth != 0))
                {
                    curY = 0;
                    curX += FMath::CeilToFloat(maxWidth) + ColumnGap;
                    maxWidth = 0;
                    j = 0;
                }
                child->SetPosition(FVector2D(curX, curY));
                curY += child->GetHeight();
                if (curY > maxHeight)
                    maxHeight = curY;
                if (child->GetWidth() > maxWidth)
                    maxWidth = child->GetWidth();
                j++;
            }
            cw = curX + FMath::CeilToFloat(maxWidth);
            ch = FMath::CeilToFloat(maxHeight);
        }
    }
    else //Pagination
    {
        int32 page = 0;
        int32 k = 0;
        float eachHeight = 0;
        if (bAutoResizeItem && LineCount > 0)
            eachHeight = FMath::FloorToFloat((viewHeight - (LineCount - 1) * LineGap) / LineCount);

        if (bAutoResizeItem && ColumnCount > 0)
        {
            float lineSize = 0;
            int32 lineStart = 0;
            float ratio;

            for (i = 0; i < cnt; i++)
            {
                child = GetChildAt(i);
                if (bFoldInvisibleItems && !child->IsVisible())
                    continue;

                if (j == 0 && ((LineCount != 0 && k >= LineCount) || (LineCount == 0 && curY + (LineCount > 0 ? eachHeight : child->GetHeight()) > viewHeight)))
                {
                    page++;
                    curY = 0;
                    k = 0;
                }

                lineSize += child->SourceSize.X;
                j++;
                if (j == ColumnCount || i == cnt - 1)
                {
                    ratio = (viewWidth - lineSize - (j - 1) * ColumnGap) / lineSize;
                    curX = 0;
                    for (j = lineStart; j <= i; j++)
                    {
                        child = GetChildAt(j);
                        if (bFoldInvisibleItems && !child->IsVisible())
                            continue;

                        child->SetPosition(FVector2D(page * viewWidth + curX, curY));

                        if (j < i)
                        {
                            child->SetSize(FVector2D(child->SourceSize.X + FMath::RoundToFloat(child->SourceSize.X * ratio),
                                LineCount > 0 ? eachHeight : child->GetHeight()), true);
                            curX += FMath::CeilToFloat(child->GetWidth()) + ColumnGap;
                        }
                        else
                        {
                            child->SetSize(FVector2D(viewWidth - curX, LineCount > 0 ? eachHeight : child->GetHeight()), true);
                        }
                        if (child->GetHeight() > maxHeight)
                            maxHeight = child->GetHeight();
                    }
                    curY += FMath::CeilToFloat(maxHeight) + LineGap;
                    maxHeight = 0;
                    j = 0;
                    lineStart = i + 1;
                    lineSize = 0;

                    k++;
                }
            }
        }
        else
        {
            for (i = 0; i < cnt; i++)
            {
                child = GetChildAt(i);
                if (bFoldInvisibleItems && !child->IsVisible())
                    continue;

                if (curX != 0)
                    curX += ColumnGap;

                if (bAutoResizeItem && LineCount > 0)
                    child->SetSize(FVector2D(child->GetWidth(), eachHeight), true);

                if ((ColumnCount != 0 && j >= ColumnCount) || (ColumnCount == 0 && curX + child->GetWidth() > viewWidth && maxHeight != 0))
                {
                    curX = 0;
                    curY += maxHeight + LineGap;
                    maxHeight = 0;
                    j = 0;
                    k++;

                    if ((LineCount != 0 && k >= LineCount) || (LineCount == 0 && curY + child->GetHeight() > viewHeight && maxWidth != 0)) //new page
                    {
                        page++;
                        curY = 0;
                        k = 0;
                    }
                }
                child->SetPosition(FVector2D(page * viewWidth + curX, curY));
                curX += FMath::CeilToFloat(child->GetWidth());
                if (curX > maxWidth)
                    maxWidth = curX;
                if (child->GetHeight() > maxHeight)
                    maxHeight = child->GetHeight();
                j++;
            }
        }
        ch = page > 0 ? viewHeight : (curY + FMath::CeilToFloat(maxHeight));
        cw = (page + 1) * viewWidth;
    }

    HandleAlign(cw, ch);
    SetBounds(0, 0, cw, ch);
}

void UGList::SetupBeforeAdd(FByteBuffer* Buffer, int32 BeginPos)
{
    UGComponent::SetupBeforeAdd(Buffer, BeginPos);

    Buffer->Seek(BeginPos, 5);

    Layout = (EListLayoutType)Buffer->ReadByte();
    SelectionMode = (EListSelectionMode)Buffer->ReadByte();
    Align = (EAlignType)Buffer->ReadByte();
    VerticalAlign = (EVerticalAlignType)Buffer->ReadByte();
    LineGap = Buffer->ReadShort();
    ColumnGap = Buffer->ReadShort();
    LineCount = Buffer->ReadShort();
    ColumnCount = Buffer->ReadShort();
    bAutoResizeItem = Buffer->ReadBool();
    ChildrenRenderOrder = (EChildrenRenderOrder)Buffer->ReadByte();
    ApexIndex = Buffer->ReadShort();

    if (Buffer->ReadBool())
    {
        Margin.Top = Buffer->ReadInt();
        Margin.Bottom = Buffer->ReadInt();
        Margin.Left = Buffer->ReadInt();
        Margin.Right = Buffer->ReadInt();
    }

    EOverflowType overflow = (EOverflowType)Buffer->ReadByte();
    if (overflow == EOverflowType::Scroll)
    {
        int32 savedPos = Buffer->GetPos();
        Buffer->Seek(BeginPos, 7);
        SetupScroll(Buffer);
        Buffer->SetPos(savedPos);
    }
    else
        SetupOverflow(overflow);

    if (Buffer->ReadBool()) //clipSoftness
        Buffer->Skip(8);

    if (Buffer->Version >= 2)
    {
        bScrollItemToViewOnClick = Buffer->ReadBool();
        bFoldInvisibleItems = Buffer->ReadBool();
    }

    Buffer->Seek(BeginPos, 8);

    DefaultItem = Buffer->ReadS();
    ReadItems(Buffer);
}

void UGList::ReadItems(FByteBuffer* Buffer)
{
    const FString* str;

    int32 itemCount = Buffer->ReadShort();
    for (int32 i = 0; i < itemCount; i++)
    {
        int32 nextPos = Buffer->ReadShort();
        nextPos += Buffer->GetPos();

        str = Buffer->ReadSP();
        if (!str || (*str).IsEmpty())
        {
            str = &DefaultItem;
            if ((*str).IsEmpty())
            {
                Buffer->SetPos(nextPos);
                continue;
            }
        }

        UGObject* obj = GetFromPool(*str);
        if (obj != nullptr)
        {
            AddChild(obj);
            SetupItem(Buffer, obj);
        }

        Buffer->SetPos(nextPos);
    }
}

void UGList::SetupItem(FByteBuffer* Buffer, UGObject* Obj)
{
    const FString* str;
    UGButton* btn = Cast<UGButton>(Obj);

    if ((str = Buffer->ReadSP()) != nullptr)
        Obj->SetText(*str);
    if ((str = Buffer->ReadSP()) != nullptr && btn)
        btn->SetSelectedTitle(*str);
    if ((str = Buffer->ReadSP()) != nullptr)
        Obj->SetIcon(*str);
    if ((str = Buffer->ReadSP()) != nullptr && btn)
        btn->SetSelectedIcon(*str);
    if ((str = Buffer->ReadSP()) != nullptr)
        Obj->Name = *str;

    UGComponent* gcom = Cast<UGComponent>(Obj);
    if (gcom != nullptr)
    {
        int32 cnt = Buffer->ReadShort();
        for (int32 i = 0; i < cnt; i++)
        {
            UGController* cc = gcom->GetController(Buffer->ReadS());
            const FString& PageID = Buffer->ReadS();
            cc->SetSelectedPageID(PageID);
        }

        if (Buffer->Version >= 2)
        {
            cnt = Buffer->ReadShort();
            for (int32 i = 0; i < cnt; i++)
            {
                const FString& target = Buffer->ReadS();
                EObjectPropID PropID = (EObjectPropID)Buffer->ReadShort();
                FString value = Buffer->ReadS();
                UGObject* obj2 = gcom->GetChildByPath(target);
                if (obj2 != nullptr)
                    obj2->SetProp(PropID, FNVariant(value));
            }
        }
    }
}

void UGList::SetupAfterAdd(FByteBuffer* Buffer, int32 BeginPos)
{
    UGComponent::SetupAfterAdd(Buffer, BeginPos);

    Buffer->Seek(BeginPos, 6);

    int32 i = Buffer->ReadShort();
    if (i != -1)
        SelectionController = Parent->GetControllerAt(i);
}