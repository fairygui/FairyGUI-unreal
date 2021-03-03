#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/IInputProcessor.h"
#include "FairyCommons.h"
#include "Event/EventContext.h"
#include "Tween/TweenManager.h"
#include "UI/UIConfig.h"
#include "FairyApplication.generated.h"

class UUIPackage;
class UNTexture;
class UGObject;
class UGRoot;
class UDragDropManager;

UCLASS(BlueprintType)
class FAIRYGUI_API UFairyApplication : public UObject
{
    GENERATED_BODY()

private:
    struct FTouchInfo
    {
        int32 UserIndex;
        int32 PointerIndex;
        bool bDown;
        bool bToClearCaptors;
        FVector2D DownPosition;
        bool bClickCancelled;
        int32 ClickCount;
        TArray<TWeakPtr<SWidget>> DownPath;
        TArray<TWeakObjectPtr<UGObject>> MouseCaptors;
        FPointerEvent Event;

        FTouchInfo();
    };

    class FInputProcessor : public IInputProcessor
    {
    public:
        virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override;
        virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
        virtual bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
        virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
    };

public:
    UFUNCTION(BlueprintPure, Category = "FairyGUI", meta = (DisplayName = "Get Application"))
    static UFairyApplication* Get();

    static void Destroy();
    static bool IsStarted() { return Instance != nullptr; }

    UFairyApplication();

    UFUNCTION(BlueprintPure, Category = "FairyGUI")
    FVector2D GetTouchPosition(int32 InUserIndex = -1, int32 InPointerIndex = -1);

    UFUNCTION(BlueprintPure, Category = "FairyGUI")
    int32 GetTouchCount() const;

    UFUNCTION(BlueprintPure, Category = "FairyGUI")
    UGObject* GetObjectUnderPoint(const FVector2D& ScreenspacePosition);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void CancelClick(int32 InUserIndex = -1, int32 InPointerIndex = -1);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void PlaySound(const FString& URL, float VolumeScale = 1);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsSoundEnabled() const { return bSoundEnabled; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetSoundEnabled(bool InEnabled);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    float GetSoundVolumeScale() const { return SoundVolumeScale; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetSoundVolumeScale(float InVolumeScale);

    bool DispatchEvent(const FName& EventType, const TSharedRef<SWidget>& Initiator, const FNVariant& Data = FNVariant::Null);
    void BubbleEvent(const FName& EventType, const TSharedRef<SWidget>& Initiator, const FNVariant& Data = FNVariant::Null);
    void BroadcastEvent(const FName& EventType, const TSharedRef<SWidget>& Initiator, const FNVariant& Data = FNVariant::Null);

    void AddMouseCaptor(int32 InUserIndex, int32 InPointerIndex, UGObject* InTarget);
    void RemoveMouseCaptor(int32 InUserIndex, int32 InPointerIndex, UGObject* InTarget);
    bool HasMouseCaptor(int32 InUserIndex, int32 InPointerIndex);

    FReply OnWidgetMouseButtonDown(const TSharedRef<SWidget>& Widget, const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
    FReply OnWidgetMouseButtonUp(const TSharedRef<SWidget>& Widget, const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
    FReply OnWidgetMouseMove(const TSharedRef<SWidget>& Widget, const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
    FReply OnWidgetMouseButtonDoubleClick(const TSharedRef<SWidget>& Widget, const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
    void OnWidgetMouseEnter(const TSharedRef<SWidget>& Widget, const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
    void OnWidgetMouseLeave(const TSharedRef<SWidget>& Widget, const FPointerEvent& MouseEvent);
    FReply OnWidgetMouseWheel(const TSharedRef<SWidget>& Widget, const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);

    UGObject* GetWidgetGObject(const TSharedPtr<SWidget>& InWidget);

    UGameViewportClient* GetViewportClient() const { return ViewportClient; }
    const TSharedPtr<SWidget>& GetViewportWidget() const { return ViewportWidget; }

    void CallAfterSlateTick(FSimpleDelegate Callback);

private:
    void OnCreate();
    void OnDestroy();

    void PreviewDownEvent(const FPointerEvent& MouseEvent);
    void PreviewUpEvent(const FPointerEvent& MouseEvent);
    void PreviewMoveEvent(const FPointerEvent& MouseEvent);

    void GetDescendants(const TSharedRef<SWidget>& InWidget, TArray<UGObject*>& OutArray);
    void GetPathToRoot(const TSharedRef<SWidget>& InWidget, TArray<UGObject*>& OutArray);

    UEventContext* BorrowEventContext();
    void ReturnEventContext(UEventContext* Context);

    void InternalBubbleEvent(const FName& EventType, const TArray<UGObject*>& CallChain, const FNVariant& Data);

    FTouchInfo* GetTouchInfo(const FPointerEvent& MouseEvent);
    FTouchInfo* GetTouchInfo(int32 InUserIndex, int32 InPointerIndex);

    void OnSlatePostTick(float DeltaTime);

private:
    UPROPERTY(Transient)
    TArray<UUIPackage*> PackageList;
    UPROPERTY(Transient)
    UGRoot* UIRoot;
    UPROPERTY(Transient)
    UDragDropManager* DragDropManager;
    UPROPERTY(Transient)
    TArray<UEventContext*> EventContextPool;

    FTweenManager TweenManager;
    TSharedPtr<IInputProcessor> InputProcessor;
    UGameViewportClient* ViewportClient;
    TSharedPtr<SWidget> ViewportWidget;
    TIndirectArray<FTouchInfo> Touches;
    FTouchInfo* LastTouch;
    bool bNeedCheckPopups;
    FDelegateHandle PostTickDelegateHandle;
    FSimpleMulticastDelegate PostTickMulticastDelegate;
    bool bSoundEnabled;
    float SoundVolumeScale;

    static UFairyApplication* Instance;

    friend class UUIPackage;
    friend class UGRoot;
    friend class FGTween;
    friend class UDragDropManager;
};