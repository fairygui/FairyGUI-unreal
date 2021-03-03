#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Slate.h"

class UGObject;

class FAIRYGUI_API SDisplayObject : public SWidget
{
public:
    SLATE_BEGIN_ARGS(SDisplayObject) :
        _GObject(nullptr),
        _Tag(SDisplayObject::SDisplayObjectTag)
    {
    }
    SLATE_ARGUMENT(UGObject*, GObject)
        SLATE_ARGUMENT(FName, Tag)
        SLATE_END_ARGS()

        static FName SDisplayObjectTag;

    SDisplayObject();
    void Construct(const FArguments& InArgs);

    const FVector2D& GetPosition() const;
    void SetPosition(const FVector2D& InPosition);
    void SetX(float InX);
    void SetY(float InY);

    void SetSize(const FVector2D& InSize);
    FVector2D GetSize();

    void SetVisible(bool bInVisible);
    bool IsVisible() const { return bVisible; }

    void SetTouchable(bool bInTouchable);
    bool IsTouchable() const { return bTouchable; }

    void SetOpaque(bool bInOpaque);
    bool IsOpaque() const { return bOpaque; }

    void SetInteractable(bool bInInteractable);
    virtual bool IsInteractable() const override { return bInteractable; }

    virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

    bool OnStage() const;

    TWeakObjectPtr<class UGObject> GObject;

protected:
    virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
    virtual FChildren* GetChildren() override;
    virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
    virtual void EnsureSizeCorrect();

    void UpdateVisibilityFlags();

protected:
    uint8 bVisible : 1;
    uint8 bInteractable : 1;
    uint8 bTouchable : 1;
    uint8 bOpaque : 1;
    FVector2D Size;

private:
    static FNoChildren NoChildrenInstance;
};