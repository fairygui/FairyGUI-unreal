#pragma once

#include "GComponent.h"
#include "GList.generated.h"

class FGObjectPool;

DECLARE_DELEGATE_TwoParams(FListItemRenderer, int32, UGObject*);
DECLARE_DELEGATE_RetVal_OneParam(FString, FListItemProvider, int32);

DECLARE_DYNAMIC_DELEGATE_TwoParams(FDynListItemRenderer, int32, Index, UGObject*, Obj);
DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(FString, FDynListItemProvider, int32, Index);

UCLASS(BlueprintType, NotBlueprintable)
class FAIRYGUI_API UGList : public UGComponent
{
    GENERATED_BODY()

public:
    UGList();
    virtual ~UGList();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    const FString& GetDefaultItem() const { return DefaultItem; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetDefaultItem(const FString& InDefaultItem);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    EListLayoutType GetLayout() const { return Layout; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetLayout(EListLayoutType InLayout);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 GetLineCount() const { return LineCount; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetLineCount(int32 InLineCount);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 GetColumnCount() { return ColumnCount; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetColumnCount(int32 InColumnCount);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 GetColumnGap() const { return ColumnGap; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetColumnGap(int32 InColumnGap);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 GetLineGap() const { return LineGap; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetLineGap(int32 InLineGap);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    EAlignType GetAlign() const { return Align; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetAlign(EAlignType InAlign);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    EVerticalAlignType GetVerticalAlign() const { return VerticalAlign; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetVerticalAlign(EVerticalAlignType InVerticalAlign);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool GetAutoResizeItem() const { return bAutoResizeItem; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetAutoResizeItem(bool bFlag);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    EListSelectionMode GetSelectionMode() const { return SelectionMode; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetSelectionMode(EListSelectionMode InMode) { SelectionMode = InMode; }

    FGObjectPool* GetItemPool() const { return Pool; }
    UGObject* GetFromPool();
    UGObject* GetFromPool(const FString& URL);
    void ReturnToPool(UGObject* Obj);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGObject* AddItemFromPool(const FString& URL = "");

    virtual UGObject* AddChildAt(UGObject* Child, int32 Index) override;
    virtual void RemoveChildAt(int32 Index) override;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void RemoveChildToPoolAt(int32 Index);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void RemoveChildToPool(UGObject* Child);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void RemoveChildrenToPool(int32 BeginIndex = 0, int32 EndIndex = -1);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 GetSelectedIndex() const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetSelectedIndex(int32 Index);

    void GetSelection(TArray<int32>& OutIndice) const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void AddSelection(int32 Index, bool bScrollItToView);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void RemoveSelection(int32 Index);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void ClearSelection();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SelectAll();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SelectReverse();

    void HandleArrowKey(int32 Direction);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void ResizeToFit(int32 ItemCount, int32 InMinSize = 0);

    virtual int32 GetFirstChildInView() const override;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void ScrollToView(int32 Index, bool bAnimation = false, bool bSetFirst = false);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGController* GetSelectionController() const { return SelectionController; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetSelectionController(UGController* InController);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetVirtual();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetVirtualAndLoop();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsVirtual() const { return bVirtual; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void RefreshVirtualList();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 GetNumItems() const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetNumItems(int32 InNumItems);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 ChildIndexToItemIndex(int32 Index) const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 ItemIndexToChildIndex(int32 Index) const;

    virtual FVector2D GetSnappingPosition(const FVector2D& InPoint) override;

    void SetItemRenderer(const FListItemRenderer& InItemRenderer) { ItemRenderer = InItemRenderer; }
    void SetItemProvider(const FListItemProvider& InItemProvider) { ItemProvider = InItemProvider; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetItemRenderer(const FDynListItemRenderer& InItemRenderer)
    { 
        if (InItemRenderer.IsBound())
            ItemRenderer = FListItemRenderer::CreateUFunction(const_cast<UObject*>(InItemRenderer.GetUObject()), InItemRenderer.GetFunctionName());
        else
            ItemRenderer.Unbind();
    }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetItemProvider(const FDynListItemProvider& InItemProvider)
    { 
        if (InItemProvider.IsBound())
            ItemProvider = FListItemProvider::CreateUFunction(const_cast<UObject*>(InItemProvider.GetUObject()), InItemProvider.GetFunctionName());
        else
            ItemProvider.Unbind();
    }

    UPROPERTY(BlueprintAssignable, Category = "FairyGUI|Event")
    FGUIEventDynMDelegate OnClickItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    bool bScrollItemToViewOnClick;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    bool bFoldInvisibleItems;

protected:
    virtual void HandleControllerChanged(UGController* Controller) override;
    virtual void HandleSizeChanged() override;
    virtual void UpdateBounds() override;
    virtual void SetupBeforeAdd(FByteBuffer* Buffer, int32 BeginPos) override;
    virtual void SetupAfterAdd(FByteBuffer* Buffer, int32 BeginPos) override;

    virtual void DispatchItemEvent(UGObject* Obj, UEventContext* Context);
    virtual void ReadItems(FByteBuffer* Buffer);
    virtual void SetupItem(FByteBuffer* Buffer, UGObject* Obj);

private:
    void ClearSelectionExcept(UGObject* Obj);
    void SetSelectionOnEvent(UGObject* Obj, UEventContext* Context);

    UFUNCTION()
    void OnClickItemHandler(UEventContext* Context);

    void UpdateSelectionController(int32 Index);

    void SetVirtual(bool bLoop);
    void CheckVirtualList();
    void SetVirtualListChangedFlag(bool bLayoutChanged);
    void DoRefreshVirtualList();

    void OnScrollHandler(UEventContext* Context);

    int32 GetIndexOnPos1(float& pos, bool forceUpdate);
    int32 GetIndexOnPos2(float& pos, bool forceUpdate);
    int32 GetIndexOnPos3(float& pos, bool forceUpdate);

    void HandleScroll(bool forceUpdate);
    bool HandleScroll1(bool forceUpdate);
    bool HandleScroll2(bool forceUpdate);
    void HandleScroll3(bool forceUpdate);

    void HandleArchOrder1();
    void HandleArchOrder2();

    void HandleAlign(float InContentWidth, float InContentHeight);

    FString DefaultItem;
    EListLayoutType Layout;
    int32 LineCount;
    int32 ColumnCount;
    int32 LineGap;
    int32 ColumnGap;
    EAlignType Align;
    EVerticalAlignType VerticalAlign;
    bool bAutoResizeItem;
    EListSelectionMode SelectionMode;
    UGController* SelectionController;
    FListItemRenderer ItemRenderer;
    FListItemProvider ItemProvider;

    FGObjectPool* Pool;
    int32 LastSelectedIndex;

    //Virtual List support
    bool bVirtual;
    bool bLoop;
    int32 NumItems;
    int32 RealNumItems;
    int32 FirstIndex;        //the top left index
    int32 CurLineItemCount;  //item count in one line
    int32 CurLineItemCount2; //item count in vertical direction,only pagination layout
    FVector2D ItemSize;
    int32 VirtualListChanged; //1-content changed, 2-size changed
    bool bEventLocked;
    uint32 ItemInfoVer;
    FTimerHandle RefreshTimerHandle;

    struct FItemInfo
    {
        FVector2D Size;
        UGObject* Obj;
        uint32 UpdateFlag;
        bool bSelected;

        FItemInfo();
    };
    TArray<FItemInfo> VirtualItems;
};