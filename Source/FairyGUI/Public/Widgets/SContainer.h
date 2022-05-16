#pragma once

#include "SDisplayObject.h"

class FAIRYGUI_API SContainer : public SDisplayObject
{
public:
    
    class FSlot : public TSlotBase<FSlot>
    {
    public:	
        SLATE_SLOT_BEGIN_ARGS(FSlot, TSlotBase<FSlot>)
        SLATE_SLOT_END_ARGS()

        FSlot(const TSharedRef<SWidget>& InWidget);
        
        FSlot(const FChildren& InParent);

        FSlot()
        {
        }

        void Construct(const FChildren& SlotOwner, FSlotArguments&& InArg);
        
    };
    
    static FSlot::FSlotArguments Slot(const TSharedRef<SWidget>& InWidget)
    {
        return FSlot::FSlotArguments(MakeUnique<FSlot>(InWidget));
    }
    
    using SContainerSlotArguments = TPanelChildren<FSlot>::FScopedWidgetSlotArguments;
    
    SContainerSlotArguments AddSlot(const TSharedRef<SWidget>& InWidget)
    {
        return InsertSlot(InWidget,INDEX_NONE);
    }
    
    SContainerSlotArguments InsertSlot(const TSharedRef<SWidget>& InWidget,int32 Index = INDEX_NONE)
    {
        return SContainerSlotArguments{MakeUnique<FSlot>(InWidget), this->Children, Index};
    }

    SLATE_BEGIN_ARGS(SContainer) 
       : _GObject(nullptr)
    {
        _Visibility = EVisibility::SelfHitTestInvisible;
    }
    SLATE_ARGUMENT(UGObject*, GObject)
    SLATE_SLOT_ARGUMENT( SContainer::FSlot, Slots )
    SLATE_END_ARGS()

    SContainer();

    void Construct(const FArguments& InArgs);

    void AddChild(const TSharedRef<SWidget>& SlotWidget);
    void AddChildAt(const TSharedRef<SWidget>& SlotWidget, int32 Index);
    int32 GetChildIndex(const TSharedRef<SWidget>& SlotWidget) const;
    void SetChildIndex(const TSharedRef<SWidget>& SlotWidget, int32 Index);
    void RemoveChild(const TSharedRef<SWidget>& SlotWidget);
    void RemoveChildAt(int32 Index);
    void RemoveChildren(int32 BeginIndex = 0, int32 EndIndex = -1);
    int32 NumChildren() const;

public:
    virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
    virtual FChildren* GetChildren() override;

protected:
    TPanelChildren<FSlot> Children;
};