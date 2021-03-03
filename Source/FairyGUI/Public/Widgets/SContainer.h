#pragma once

#include "SDisplayObject.h"

class FAIRYGUI_API SContainer : public SDisplayObject
{
public:
    SLATE_BEGIN_ARGS(SContainer) :
        _GObject(nullptr)
    {}
    SLATE_ARGUMENT(UGObject*, GObject)
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
    TPanelChildren<FSlotBase> Children;
};