#pragma once

#include "CoreMinimal.h"
#include "GTweener.h"
#include "EaseType.h"
#include "TweenValue.h"

class FAIRYGUI_API FGTween
{
public:
    static FGTweener* To(float StartValue, float EndValue, float Duration);
    static FGTweener* To(const FVector2D& StartValue, const FVector2D& EndValue, float Duration);
    static FGTweener* To(const FVector& StartValue, const FVector& EndValue, float Duration);
    static FGTweener* To(const FVector4& StartValue, const FVector4& EndValue, float Duration);
    static FGTweener* To(const FColor& StartValue, const FColor& EndValue, float Duration);
    static FGTweener* ToDouble(double StartValue, double EndValue, float Duration);
    static FGTweener* DelayedCall(float Delay);
    static FGTweener* Shake(const FVector2D& StartValue, float Amplitude, float Duration);

    static bool IsTweening(const FTweenerHandle& Handle);
    static bool IsTweening(UObject* Target);

    static FGTweener* GetTween(const FTweenerHandle& Handle);
    static FGTweener* GetTween(UObject* Target);

    static void Kill(FTweenerHandle& Handle, bool bSetComplete = false);
    static void Kill(UObject* Target, bool bSetComplete = false);
};

class FAIRYGUI_API FGTweenAction
{
public:
    static void MoveX(FGTweener* Tweener);
    static void MoveY(FGTweener* Tweener);
    static void Move(FGTweener* Tweener);
    static void SetWidth(FGTweener* Tweener);
    static void SetHeight(FGTweener* Tweener);
    static void SetSize(FGTweener* Tweener);
    static void ScaleX(FGTweener* Tweener);
    static void ScaleY(FGTweener* Tweener);
    static void ScaleXY(FGTweener* Tweener);
    static void Rotate(FGTweener* Tweener);
    static void SetAlpha(FGTweener* Tweener);
    static void SetProgress(FGTweener* Tweener);
};
