#pragma once

#include "CoreMinimal.h"
#include "EaseType.h"
#include "TweenValue.h"
#include "TweenerHandle.h"
#include "Utils/NVariant.h"

class FGPath;
class FGTweener;

DECLARE_DELEGATE_OneParam(FTweenDelegate, FGTweener*);

class FAIRYGUI_API FGTweener
{
public:
    FGTweener();
    ~FGTweener();

    const FTweenerHandle& GetHandle() {
        return Handle;
    }

    FGTweener* SetDelay(float InValue);
    float GetDelay() const { return Delay; }
    FGTweener* SetDuration(float InValue);
    float GetDuration() const { return Duration; }
    FGTweener* SetBreakpoint(float InValue);
    FGTweener* SetEase(EEaseType InValue);
    FGTweener* SetEasePeriod(float InValue);
    FGTweener* SetEaseOvershootOrAmplitude(float InValue);
    FGTweener* SetRepeat(int32 InRepeat, bool bInYoyo = false);
    int32 GetRepeat() const { return Repeat; }
    FGTweener* SetTimeScale(float InValue);
    FGTweener* SetSnapping(bool InValue);
    FGTweener* SetTarget(UObject* InTarget);
    UObject* GetTarget() const { return Target.Get(); }
    const FNVariant& GetUserData() const { return UserData; }
    FGTweener* SetUserData(const FNVariant& InData);
    FGTweener* SetPath(TSharedPtr<FGPath> InPath);
    FGTweener* OnUpdate(FTweenDelegate Callback);
    FGTweener* OnStart(FTweenDelegate Callback);
    FGTweener* OnComplete(FTweenDelegate Callback);
    FGTweener* OnUpdate(FSimpleDelegate Callback);
    FGTweener* OnStart(FSimpleDelegate Callback);
    FGTweener* OnComplete(FSimpleDelegate Callback);

    float GetNormalizedTime() const { return NormalizedTime; }
    bool IsCompleted() const { return Ended != 0; }
    bool AllCompleted() const { return Ended == 1; }
    FGTweener* SetPaused(bool bInPaused);
    void Seek(float Time);
    void Kill(bool bSetComplete = false);

    FTweenValue StartValue;
    FTweenValue EndValue;
    FTweenValue Value;
    FTweenValue DeltaValue;

private:
    FGTweener* To(float InStart, float InEnd, float InDuration);
    FGTweener* To(const FVector2D& InStart, const FVector2D& InEnd, float InDuration);
    FGTweener* To(const FVector& InStart, const FVector& InEnd, float InDuration);
    FGTweener* To(const FVector4& InStart, const FVector4& InEnd, float InDuration);
    FGTweener* To(const FColor& InStart, const FColor& InEnd, float InDuration);
    FGTweener* To(double InStart, double InEnd, float InDuration);
    FGTweener* Shake(const FVector2D& InStart, float InAmplitude, float InDuration);
    void Init();
    void Reset();
    void Update(float DeltaTime);
    void Update();

private:
    TWeakObjectPtr<UObject> Target;
    bool bKilled;
    bool bPaused;

    float Delay;
    float Duration;
    float Breakpoint;
    EEaseType EaseType;
    float EaseOvershootOrAmplitude;
    float EasePeriod;
    int32 Repeat;
    bool bYoyo;
    float TimeScale;
    bool bSnapping;
    FNVariant UserData;
    int32 ValueSize;
    TSharedPtr<FGPath> Path;
    FTweenerHandle Handle;

    FTweenDelegate OnUpdateCallback;
    FTweenDelegate OnStartCallback;
    FTweenDelegate OnCompleteCallback;

    bool bStarted;
    int32 Ended;
    float ElapsedTime;
    float NormalizedTime;

    friend class FGTween;
    friend class FTweenManager;
};
