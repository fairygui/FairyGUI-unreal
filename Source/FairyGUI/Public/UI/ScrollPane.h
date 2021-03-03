#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateTypes.h"
#include "FieldTypes.h"
#include "Tween/GTween.h"
#include "Event/EventContext.h"
#include "ScrollPane.generated.h"

class UGObject;
class UGComponent;
class UGScrollBar;
class UGController;
class FByteBuffer;
class FGTweener;
class SContainer;

UCLASS(BlueprintType)
class FAIRYGUI_API UScrollPane : public UObject
{
    GENERATED_BODY()
public:
    UScrollPane();
    ~UScrollPane();

    void Setup(FByteBuffer* Buffer);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGComponent* GetHeader() const { return Header; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGComponent* GetFooter() const { return Footer; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGScrollBar* GetVtScrollBar() const { return VtScrollBar; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGScrollBar* GetHzScrollBar() const { return HzScrollBar; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    float GetPosX() const { return XPos; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetPosX(float Value, bool bAnimation = false);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    float GetPosY() const { return YPos; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetPosY(float Value, bool bAnimation = false);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    float GetPercX() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetPercX(float Value, bool bAnimation = false);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    float GetPercY() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetPercY(float Value, bool bAnimation = false);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsBottomMost() const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsRightMost() const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void ScrollLeft(float Ratio = 1, bool bAnimation = false);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void ScrollRight(float Ratio = 1, bool bAnimation = false);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void ScrollUp(float Ratio = 1, bool bAnimation = false);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void ScrollDown(float Ratio = 1, bool bAnimation = false);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void ScrollTop(bool bAnimation = false);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void ScrollBottom(bool bAnimation = false);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void ScrollToView(UGObject* Obj, bool bAnimation = false, bool bSetFirst = false);

    void ScrollToView(const FBox2D& Rect, bool bAnimation = false, bool bSetFirst = false);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsChildInView(UGObject* Obj) const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 GetPageX() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetPageX(int32 PageX, bool bAnimation = false);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 GetPageY() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetPageY(int32 PageY, bool bAnimation = false);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    float GetScrollingPosX() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    float GetScrollingPosY() const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    const FVector2D& GetContentSize() const { return ContentSize; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    const FVector2D& GetViewSize() const { return ViewSize; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void LockHeader(int32 Size);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void LockFooter(int32 Size);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void CancelDragging();

    static UScrollPane* GetDraggingPane() { return DraggingPane.Get(); }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    uint8 bBouncebackEffect : 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    uint8 bTouchEffect : 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    uint8 bInertiaDisabled : 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    uint8 bMouseWheelEnabled : 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    uint8 bSnapToItem : 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    uint8 bPageMode : 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    float DecelerationRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    float ScrollStep;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    UGController* PageController;

private:
    void OnOwnerSizeChanged();
    void AdjustMaskContainer();
    void SetContentSize(const FVector2D& InSize);
    void ChangeContentSizeOnScrolling(float DeltaWidth, float DeltaHeight, float DeltaPosX, float DeltaPosY);
    void SetViewWidth(float Width);
    void SetViewHeight(float Height);
    void SetSize(const FVector2D& InSize);
    void HandleSizeChanged();

    void HandleControllerChanged(UGController* Controller);
    void UpdatePageController();

    void PosChanged(bool bAnimation);
    void Refresh();
    void Refresh2();

    void UpdateScrollBarPos();
    void UpdateScrollBarVisible();
    void UpdateScrollBarVisible2(UGScrollBar* Bar);

    float GetLoopPartSize(float Division, int32 Axis);
    bool LoopCheckingCurrent();
    void LoopCheckingTarget(FVector2D& EndPos);
    void LoopCheckingTarget(FVector2D& EndPos, int32 Axis);
    void LoopCheckingNewPos(float& Value, int32 Axis);
    void AlignPosition(FVector2D& Pos, bool bInertialScrolling);
    float AlignByPage(float Pos, int32 Axis, bool bInertialScrolling);
    FVector2D UpdateTargetAndDuration(const FVector2D& OrignPos);
    float UpdateTargetAndDuration(float Pos, int32 Axis);
    void FixDuration(int32 Axis, float DldChange);
    void StartTween(int32 Type);
    void KillTween();
    void TweenUpdate();
    float RunTween(int32 Axis, float Delta);

    void CheckRefreshBar();

    void OnTouchBegin(UEventContext* Context);
    void OnTouchMove(UEventContext* Context);
    void OnTouchEnd(UEventContext* Context);
    void OnMouseWheel(UEventContext* Context);
    void OnRollOver(UEventContext* Context);
    void OnRollOut(UEventContext* Context);

    void OnBarTweenComplete(FGTweener* Tweener);

private:
    UGComponent* Owner;
    TSharedPtr<SContainer> MaskContainer;
    TSharedPtr<SContainer> Container;
    UPROPERTY(Transient)
    UGScrollBar* HzScrollBar;
    UPROPERTY(Transient)
    UGScrollBar* VtScrollBar;
    UPROPERTY(Transient)
    UGComponent* Header;
    UPROPERTY(Transient)
    UGComponent* Footer;

    EScrollType ScrollType;
    FMargin ScrollBarMargin;
    uint8 bScrollBarDisplayAuto : 1;
    uint8 bVScrollNone : 1;
    uint8 bHScrollNone : 1;
    uint8 bNeedRefresh : 1;
    int32 RefreshBarAxis;
    uint8 bDisplayOnLeft : 1;
    uint8 bDisplayInDemand : 1;
    uint8 bFloating : 1;
    uint8 bDontClipMargin : 1;

    float XPos;
    float YPos;

    FVector2D ViewSize;
    FVector2D ContentSize;
    FVector2D OverlapSize;
    FVector2D PageSize;
    FVector2D ContainerPos;
    FVector2D BeginTouchPos;
    FVector2D LastTouchPos;
    FVector2D LastTouchGlobalPos;
    FVector2D Velocity;
    float VelocityScale;
    float LastMoveTime;
    uint8 bDragged : 1;
    uint8 bIsHoldAreaDone : 1;
    int32 AniFlag;
    int32 LoopMode;
    uint8 bHover : 1;

    float HeaderLockedSize;
    float FooterLockedSize;
    uint8 bDispatchingPullDown : 1;
    uint8 bDispatchingPullUp : 1;

    int32 Tweening;
    FVector2D TweenStart;
    FVector2D TweenChange;
    FVector2D TweenTime;
    FVector2D TweenDuration;

    FTimerHandle RefreshTimerHandle;
    FTimerHandle TickTimerHandle;

    static int32 GestureFlag;
    static TWeakObjectPtr<UScrollPane> DraggingPane;

    friend class UGComponent;
    friend class UGList;
    friend class UGScrollBar;
};