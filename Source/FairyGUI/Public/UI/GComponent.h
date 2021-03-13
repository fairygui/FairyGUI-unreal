#pragma once

#include "GObject.h"
#include "ScrollPane.h"
#include "GComponent.generated.h"

class UGController;
class UTransition;
class SContainer;

UCLASS(BlueprintType, Blueprintable)
class FAIRYGUI_API UGComponent : public UGObject
{
    GENERATED_BODY()

public:
    UGComponent();
    virtual ~UGComponent();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGObject* AddChild(UGObject* Child);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    virtual UGObject* AddChildAt(UGObject* Child, int32 Index);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void RemoveChild(UGObject* Child);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    virtual void RemoveChildAt(int32 Index);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void RemoveChildren(int32 BeginIndex = 0, int32 EndIndex = -1);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI", meta = (DeterminesOutputType = "ClassType"))
    UGObject* GetChildAt(int32 Index, TSubclassOf<UGObject> ClassType = nullptr) const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI", meta = (DeterminesOutputType = "ClassType"))
    UGObject* GetChild(const FString& ChildName, TSubclassOf<UGObject> ClassType = nullptr) const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI", meta = (DeterminesOutputType = "ClassType"))
    UGObject* GetChildByPath(const FString& Path, TSubclassOf<UGObject> ClassType = nullptr) const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI", meta = (DeterminesOutputType = "ClassType"))
    UGObject* GetChildInGroup(const UGGroup* Group, const FString& ChildName, TSubclassOf<UGObject> ClassType = nullptr) const;

    UGObject* GetChildByID(const FString& ChildID) const;
    const TArray<UGObject*>& GetChildren() const { return Children; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 GetChildIndex(const UGObject* Child) const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetChildIndex(UGObject* Child, int32 Index);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 SetChildIndexBefore(UGObject* Child, int32 Index);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SwapChildren(UGObject* Child1, UGObject* Child2);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SwapChildrenAt(int32 Index1, int32 Index2);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 NumChildren() const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsAncestorOf(const UGObject* Obj) const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    virtual bool IsChildInView(UGObject* Child) const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    virtual int32 GetFirstChildInView() const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGController* GetControllerAt(int32 Index) const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGController* GetController(const FString& ControllerName) const;

    const TArray<UGController*>& GetControllers() const { return Controllers; }
    void AddController(UGController* Controller);
    void RemoveController(UGController* Controller);
    void ApplyController(UGController* Controller);
    void ApplyAllControllers();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UTransition* GetTransition(const FString& TransitionName) const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UTransition* GetTransitionAt(int32 Index) const;

    const TArray<UTransition*>& GetTransitions() const { return Transitions; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsOpaque() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetOpaque(bool bInOpaque);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    const FMargin& GetMargin() { return Margin; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetMargin(const FMargin& InMargin);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    EChildrenRenderOrder GetChildrenRenderOrder() const { return ChildrenRenderOrder; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetChildrenRenderOrder(EChildrenRenderOrder InChildrenRenderOrder);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 GetApexIndex() const { return ApexIndex; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetApexIndex(int32 InApedIndex);

    //UGObject* getMask() const;
    //void setMask(UGObject* value, bool inverted = false);

    virtual IHitTest* GetHitArea() const override { return HitArea.Get(); }
    void SetHitArea(const TSharedPtr<IHitTest>& InHitArea);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UScrollPane* GetScrollPane() const { return ScrollPane; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    float GetViewWidth() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetViewWidth(float InViewWidth);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    float GetViewHeight() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetViewHeight(float InViewHeight);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetBoundsChangedFlag();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void EnsureBoundsCorrect();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void OnClickChild(const FString& ChildName, const FGUIEventDynDelegate& Delegate)
    {
        GetChild(ChildName)->OnClick.AddUnique(Delegate);
    }

    UPROPERTY(BlueprintAssignable, Category = "FairyGUI|Event")
    FGUIEventDynMDelegate OnDrop;

    UPROPERTY(BlueprintAssignable, Category = "FairyGUI|Event")
    FGUIEventDynMDelegate OnScroll;
   
    UPROPERTY(BlueprintAssignable, Category = "FairyGUI|Event")
    FGUIEventDynMDelegate OnScrollEnd;

    UPROPERTY(BlueprintAssignable, Category = "FairyGUI|Event")
    FGUIEventDynMDelegate OnPullUpRelease;

    UPROPERTY(BlueprintAssignable, Category = "FairyGUI|Event")
    FGUIEventDynMDelegate OnPullDownRelease;

    virtual FVector2D GetSnappingPosition(const FVector2D& InPoint);

    //internal use
    void ChildSortingOrderChanged(UGObject* Child, int32 OldValue, int32 NewValue);
    void ChildStateChanged(UGObject* Child);
    void AdjustRadioGroupDepth(UGObject* Child, UGController* Controller);

    virtual void ConstructFromResource() override;
    void ConstructFromResource(TArray<UGObject*>* ObjectPool, int32 PoolIndex);

    bool bBuildingDisplayList;

protected:
    virtual void ConstructExtension(FByteBuffer* Buffer);
    virtual void OnConstruct();
    UFUNCTION(BlueprintImplementableEvent, Category = "FairyGUI",  meta = ( DisplayName = "OnConstruct"))
    void K2_OnConstruct();
    virtual void SetupAfterAdd(FByteBuffer* Buffer, int32 BeginPos) override;
    virtual void HandleSizeChanged() override;
    virtual void HandleGrayedChanged() override;
    virtual void HandleControllerChanged(UGController* Controller) override;

    virtual void UpdateBounds();
    void SetBounds(float ax, float ay, float aw, float ah);

    void SetupOverflow(EOverflowType InOverflow);
    void SetupScroll(FByteBuffer* Buffer);

    UPROPERTY(Transient)
    TArray<UGObject*> Children;
    UPROPERTY(Transient)
    TArray<UGController*> Controllers;
    UPROPERTY(Transient)
    TArray<UTransition*> Transitions;
    UPROPERTY(Transient)
    UScrollPane* ScrollPane;

    TSharedPtr<SContainer> RootContainer;
    TSharedPtr<SContainer> Container;
    FMargin Margin;
    FVector2D AlignOffset;
    EChildrenRenderOrder ChildrenRenderOrder;
    int32 ApexIndex;
    uint8 bBoundsChanged : 1;
    uint8 bTrackBounds : 1;
    TSharedPtr<IHitTest> HitArea;

private:
    int32 GetInsertPosForSortingChild(UGObject* Child);
    int32 MoveChild(UGObject* Child, int32 OldIndex, int32 NewIndex);

    void BuildNativeDisplayList(bool bImmediatelly = false);

    void OnAddedToStageHandler(UEventContext* Context);
    void OnRemovedFromStageHandler(UEventContext* Context);

    int32 SortingChildCount;
    UGController* ApplyingController;

    FTimerHandle UpdateBoundsTimerHandle;
    FTimerHandle BuildDisplayListTimerHandle;

    friend class UScrollPane;
};