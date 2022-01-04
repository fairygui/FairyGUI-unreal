#pragma once

#include "Relations.h"
#include "PackageItem.h"
#include "UIConfig.h"
#include "FairyCommons.h"
#include "Widgets/SDisplayObject.h"
#include "Widgets/HitTest.h"
#include "Event/EventContext.h"
#include "Utils/NVariant.h"
#include "GObject.generated.h"

class FByteBuffer;
class FRelations;
class FGearBase;

class UGGroup;
class UGComponent;
class UGTreeNode;
class UGController;
class UGRoot;

UCLASS(BlueprintType)
class FAIRYGUI_API UGObject : public UObject
{
    GENERATED_BODY()

public:
    UGObject();
    virtual ~UGObject();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    float GetX() const { return Position.X; };
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetX(float InX);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    float GetY() const { return Position.Y; };
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetY(float InY);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    const FVector2D& GetPosition() const { return Position; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetPosition(const FVector2D& InPosition);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    float GetXMin() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetXMin(float InXMin);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    float GetYMin() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetYMin(float InYMin);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    float GetWidth() const { return Size.X; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetWidth(float InWidth) { SetSize(FVector2D(InWidth, RawSize.Y)); }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    float GetHeight() const { return Size.Y; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetHeight(float InHeight) { SetSize(FVector2D(RawSize.X, InHeight)); }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    const FVector2D& GetSize() const { return Size; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI", meta = (AdvancedDisplay="bIgnorePivot"))
    void SetSize(const FVector2D& InSize, bool bIgnorePivot = false);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void Center(bool bRestraint = false);
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void MakeFullScreen(bool bRestraint = false);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    const FVector2D& GetPivot() const { return Pivot; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetPivot(const FVector2D& InPivot, bool bAsAnchor = false);
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsPivotAsAnchor() const { return bPivotAsAnchor; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    float GetScaleX() const { return Scale.X; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetScaleX(float InScaleX) { SetScale(FVector2D(InScaleX, Scale.Y)); }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    float GetScaleY() const { return Scale.Y; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetScaleY(float InScaleY) { SetScale(FVector2D(Scale.X, InScaleY)); }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    const FVector2D& GetScale() const { return Scale; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetScale(const FVector2D& InScale);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    const FVector2D& GetSkew() const { return Skew; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetSkew(const FVector2D& InSkew);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    float GetRotation() const { return Rotation; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetRotation(float InRotation);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    float GetAlpha() const { return Alpha; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetAlpha(float InAlpha);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsGrayed() const { return bGrayed; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetGrayed(bool bInGrayed);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsVisible() const { return bVisible; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetVisible(bool bInVisible);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsTouchable() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetTouchable(bool bInTouchable);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 GetSortingOrder() const { return SortingOrder; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetSortingOrder(int32 InSortingOrder);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGGroup* GetGroup() const { return Group.Get(); }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetGroup(UGGroup* InGroup);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    virtual const FString& GetText() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    virtual void SetText(const FString& InText);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    virtual const FString& GetIcon() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    virtual void SetIcon(const FString& InIcon);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    const FString& GetTooltips() const { return Tooltips; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetTooltips(const FString& InTooltips);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsDraggable() const { return bDraggable; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetDraggable(bool bInDraggable);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    FBox2D GetDragBounds() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetDragBounds(const FBox2D& InBounds);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void StartDrag(int32 UserIndex, int32 PointerIndex);
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void StopDrag();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    FString GetResourceURL() const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    FString GetResourceName() const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    FString GetPackageName() const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    FVector2D GlobalToLocal(const FVector2D& InPoint);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    FBox2D GlobalToLocalRect(const FBox2D& InRect);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    FVector2D RootToLocal(const FVector2D& InPoint);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    FBox2D RootToLocalRect(const FBox2D& InRect);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    FVector2D LocalToGlobal(const FVector2D& InPoint);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    FBox2D LocalToGlobalRect(const FBox2D& InRect);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    FVector2D LocalToRoot(const FVector2D& InPoint);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    FBox2D LocalToRootRect(const FBox2D& InRect);

    const TSharedPtr<FRelations>& GetRelations() { return Relations; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void AddRelation(UGObject* Obj, ERelationType RelationType, bool bUsePercent = false);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void RemoveRelation(UGObject* Obj, ERelationType RelationType);

    const TSharedPtr<FGearBase>& GetGear(int32 Index);
    bool CheckGearController(int32 Index, UGController* Controller);
    uint32 AddDisplayLock();
    void ReleaseDisplayLock(uint32 Token);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGComponent* GetParent() const { return Parent.Get(); }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetParent(UGObject* InParent);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetParentToRoot();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGRoot* GetUIRoot() const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UFairyApplication* GetApp() const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool OnStage() const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void RemoveFromParent();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI", meta = (DeterminesOutputType = "ClassType"))
    UGObject* CastTo(TSubclassOf<UGObject> ClassType) const;

    template <typename T> T* As() const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGTreeNode* GetTreeNode() const { return TreeNode; }

    TSharedPtr<FPackageItem> GetPackageItem() const { return PackageItem; }
    TSharedRef<SDisplayObject> GetDisplayObject() const { return DisplayObject.ToSharedRef(); }

    virtual IHitTest* GetHitArea() const { return nullptr; }

    template <typename T> T GetProp(EObjectPropID PropID) const;
    virtual FNVariant GetProp(EObjectPropID PropID) const;
    virtual void SetProp(EObjectPropID PropID, const FNVariant& InValue);

    virtual void ConstructFromResource();

public:
    bool DispatchEvent(const FName& EventType, const FNVariant& Data = FNVariant::Null);
    bool HasEventListener(const FName& EventType) const;
    void InvokeEventDelegate(UEventContext* Context);
    FGUIEventMDelegate& On(const FName& EventType);

    FSimpleMulticastDelegate& OnPositionChanged()
    {
        return OnPositionChangedEvent;
    }

    FSimpleMulticastDelegate& OnSizeChanged()
    {
        return OnSizeChangedEvent;
    }

    UPROPERTY(BlueprintAssignable, Category = "FairyGUI|Event")
    FGUIEventDynMDelegate OnClick;

    UPROPERTY(BlueprintAssignable, Category = "FairyGUI|Event")
    FGUIEventDynMDelegate OnTouchBegin;

    UPROPERTY(BlueprintAssignable, Category = "FairyGUI|Event")
    FGUIEventDynMDelegate OnTouchMove;

    UPROPERTY(BlueprintAssignable, Category = "FairyGUI|Event")
    FGUIEventDynMDelegate OnTouchEnd;

    UPROPERTY(BlueprintAssignable, Category = "FairyGUI|Event")
    FGUIEventDynMDelegate OnRollOver;

    UPROPERTY(BlueprintAssignable, Category = "FairyGUI|Event")
    FGUIEventDynMDelegate OnRollOut;

    UPROPERTY(BlueprintAssignable, Category = "FairyGUI|Event")
    FGUIEventDynMDelegate OnDragStart;

    UPROPERTY(BlueprintAssignable, Category = "FairyGUI|Event")
    FGUIEventDynMDelegate OnDragMove;

    UPROPERTY(BlueprintAssignable, Category = "FairyGUI|Event")
    FGUIEventDynMDelegate OnDragEnd;

    UPROPERTY(BlueprintAssignable, Category = "FairyGUI|Event")
    FGUIEventDynMDelegate OnGearStop;

    UPROPERTY(BlueprintAssignable, Category = "FairyGUI|Event")
    FGUIEventDynMDelegate OnAddedToStage;

    UPROPERTY(BlueprintAssignable, Category = "FairyGUI|Event")
    FGUIEventDynMDelegate OnRemovedFromStage;

public:
    UPROPERTY(Transient, BlueprintReadOnly, Category = "FairyGUI")
    FString ID;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    FString Name;

    UPROPERTY(Transient, BlueprintReadOnly, Category = "FairyGUI")
    FVector2D SourceSize;
    UPROPERTY(Transient, BlueprintReadOnly, Category = "FairyGUI")
    FVector2D InitSize;
    UPROPERTY(Transient, BlueprintReadWrite, Category = "FairyGUI")
    FVector2D MinSize;
    UPROPERTY(Transient, BlueprintReadWrite, Category = "FairyGUI")
    FVector2D MaxSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    FNVariant UserData;
    
    bool bUnderConstruct;
    bool bGearLocked;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    static UGObject* GetDraggingObject() { return DraggingObject.Get(); }

public:
    virtual UWorld* GetWorld() const override;

protected:
    TWeakObjectPtr<UGComponent> Parent;
    TSharedPtr<SDisplayObject> DisplayObject;
    TSharedPtr<FPackageItem> PackageItem;

    virtual void HandleSizeChanged();
    virtual void HandleGrayedChanged();
    virtual void HandlePositionChanged();
    virtual void HandleControllerChanged(UGController* Controller);
    virtual void HandleAlphaChanged();
    virtual void HandleVisibleChanged();

    virtual void SetupBeforeAdd(FByteBuffer* Buffer, int32 BeginPos);
    virtual void SetupAfterAdd(FByteBuffer* Buffer, int32 BeginPos);

    void UpdateGear(int32 Index);
    void CheckGearDisplay();

    void SetSizeDirectly(const FVector2D& InSize);

    FVector2D Position;
    FVector2D Size;
    FVector2D RawSize;
    FVector2D Pivot;
    FVector2D Scale;
    FVector2D Skew;
    uint8 bPivotAsAnchor : 1;
    float Alpha;
    float Rotation;
    uint8 bVisible : 1;
    uint8 bGrayed : 1;

private:
    bool InternalVisible() const;
    bool InternalVisible2() const;
    bool InternalVisible3() const;
    void UpdateGearFromRelations(int32 Index, const FVector2D& Delta);
    void UpdateTransform();

    UFUNCTION()
    void OnRollOverHandler(UEventContext* Context);
    UFUNCTION()
    void OnRollOutHandler(UEventContext* Context);

    void InitDrag();
    void DragBegin(int32 UserIndex, int32 PointerIndex);
    void DragEnd();

    UFUNCTION()
    void OnTouchBeginHandler(UEventContext* Context);
    UFUNCTION()
    void OnTouchMoveHandler(UEventContext* Context);
    UFUNCTION()
    void OnTouchEndHandler(UEventContext* Context);

    uint8 bInternalVisible : 1;
    uint8 bHandlingController : 1;
    uint8 bDraggable : 1;
    int32 SortingOrder;
    FString Tooltips;
    TWeakObjectPtr<UGGroup> Group;
    float SizePercentInGroup;
    TSharedPtr<FRelations> Relations;
    TSharedPtr<FGearBase> Gears[10];
    FVector2D DragTouchStartPos;
    TOptional<FBox2D> DragBounds;
    uint8 bDragTesting : 1;
    UGTreeNode* TreeNode;
    UFairyApplication* CachedApp;

    struct FUnifiedEventDelegate
    {
        FGUIEventMDelegate Func;
        FGUIEventDynMDelegate* DynFunc;
    };
    TMap<FName, FUnifiedEventDelegate> EventDelegates;
    FUnifiedEventDelegate& GetEventDelegate(const FName& EventType);

    FSimpleMulticastDelegate OnPositionChangedEvent;
    FSimpleMulticastDelegate OnSizeChangedEvent;

    static TWeakObjectPtr<UGObject> DraggingObject;
    static FVector2D GlobalDragStart;
    static FBox2D GlobalRect;
    static bool bUpdateInDragging;

    friend class UGComponent;
    friend class UGGroup;
    friend class FRelationItem;
    friend class FUIObjectFactory;
    friend class UGTree;
};

template <typename T>
inline T* UGObject::As() const
{
    return const_cast<T*>(Cast<T>(this));
}

template <typename T>
inline T UGObject::GetProp(EObjectPropID PropID) const
{
    return GetProp(PropID).As<T>();
}