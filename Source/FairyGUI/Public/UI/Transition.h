#pragma once

#include "CoreMinimal.h"
#include "Tween/GTween.h"
#include "Event/EventContext.h"
#include "Transition.generated.h"

class UGObject;
class UGComponent;
class UGController;
class FByteBuffer;
class FGTweener;
struct FTransitionItem;

UCLASS(BlueprintType)
class FAIRYGUI_API UTransition : public UObject
{
    GENERATED_BODY()
public:
    UTransition();
    virtual ~UTransition();

    bool IsPlaying() const { return bPlaying; }
    void Play(const FSimpleDelegate& InCompleteCallback = FSimpleDelegate())
    {
        Play(1, 0, 0, -1, false, InCompleteCallback);
    }

    void Play(int32 InTimes, float InDelay, float InStartTime = 0, float InEndTime = -1, const FSimpleDelegate& InCompleteCallback = FSimpleDelegate())
    {
        Play(InTimes, InDelay, InStartTime, InEndTime, false, InCompleteCallback);
    }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI", meta = (AutoCreateRefTerm = "InCompleteCallback"))
    void Play(const FSimpleDynDelegate& InCompleteCallback, int32 InTimes = 1, float InDelay = 0, float InStartTime = 0, float InEndTime = -1)
    {
        Play(InTimes, InDelay, InStartTime, InEndTime, false, 
            InCompleteCallback.IsBound() ?
            FSimpleDelegate::CreateUFunction(const_cast<UObject*>(InCompleteCallback.GetUObject()), InCompleteCallback.GetFunctionName())
            : FSimpleDelegate());
    }

    void PlayReverse(const FSimpleDelegate& InCompleteCallback = FSimpleDelegate())
    {
        Play(1, 0, 0, -1, true, InCompleteCallback);
    }

    void PlayReverse(int32 InTimes, float InDelay, const FSimpleDelegate& InCompleteCallback = FSimpleDelegate())
    {
        Play(InTimes, InDelay, 0, -1, true, InCompleteCallback);
    }
    
    UFUNCTION(BlueprintCallable, Category = "FairyGUI", meta = (AutoCreateRefTerm = "InCompleteCallback"))
    void PlayReverse(const FSimpleDynDelegate& InCompleteCallback, int32 InTimes = 1, float InDelay = 0)
    {
        Play(InTimes, InDelay, 0, -1, true, 
            InCompleteCallback.IsBound() ?
            FSimpleDelegate::CreateUFunction(const_cast<UObject*>(InCompleteCallback.GetUObject()), InCompleteCallback.GetFunctionName())
            : FSimpleDelegate());
    }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void ChangePlayTimes(int32 InTimes);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void Stop(bool bSetToComplete = false, bool bProcessCallback = false);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetAutoPlay(bool bInAutoPlay, int32 InTimes, float InDelay);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetPaused(bool bInPaused);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetValue(const FString& InLabel, const TArray<FNVariant>& InValues);

    void SetHook(const FString& InLabel, FSimpleDelegate Callback);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetHook(const FString& InLabel, FSimpleDynDelegate Callback)
    {
        SetHook(InLabel, Callback.IsBound() ?
            FSimpleDelegate::CreateUFunction(const_cast<UObject*>(Callback.GetUObject()), Callback.GetFunctionName())
            : FSimpleDelegate());
    }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void ClearHooks();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetTarget(const FString& InLabel, UGObject* InTarget);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetDuration(const FString& InLabel, float InDuration);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    float GetLabelTime(const FString& InLabel) const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    float GetTimeScale() const { return TimeScale; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetTimeScale(float InTimeScale);

    void UpdateFromRelations(const FString& TargetID, const FVector2D& Delta);
    void OnOwnerAddedToStage();
    void OnOwnerRemovedFromStage();

    void Setup(FByteBuffer* Buffer);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    FString Name;

private:
    void Play(int32 InTimes, float InDelay, float InStartTime, float InEndTime, bool bInReverse, FSimpleDelegate InCompleteCallback);
    void StopItem(FTransitionItem* Item, bool bSetToComplete);
    void OnDelayedPlay();
    void InternalPlay();
    void PlayItem(FTransitionItem* Item);
    void SkipAnimations();
    void OnDelayedPlayItem(FGTweener* Tweener);
    void OnTweenStart(FGTweener* Tweener);
    void OnTweenUpdate(FGTweener* Tweener);
    void OnTweenComplete(FGTweener* Tweener);
    void OnPlayTransCompleted(FTransitionItem* item);
    void CallHook(FTransitionItem* Item, bool bTweenEnd);
    void CheckAllComplete();
    void ApplyValue(FTransitionItem* Item);
    void DecodeValue(FTransitionItem* Item, FByteBuffer* Buffer, struct FTransitionItemData* Value);

    UGComponent* Owner;
    TArray<FTransitionItem*> Items;
    int32 TotalTimes;
    int32 TotalTasks;
    bool bPlaying;
    bool bPaused;
    FVector2D OwnerBasePos;
    FSimpleDelegate CompleteCallback;
    int32 Options;
    bool bReversed;
    float TotalDuration;
    bool bAutoPlay;
    int32 AutoPlayTimes;
    float AutoPlayDelay;
    float TimeScale;
    float StartTime;
    float EndTime;
    FTweenerHandle DelayHandle;
};