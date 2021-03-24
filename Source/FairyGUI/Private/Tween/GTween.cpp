#include "Tween/GTween.h"
#include "Tween/TweenManager.h"
#include "UI/GProgressBar.h"

FGTweener* FGTween::To(float StartValue, float EndValue, float Duration)
{
    return FTweenManager::Singleton.CreateTween()->To(StartValue, EndValue, Duration);
}

FGTweener* FGTween::To(const FVector2D& StartValue, const FVector2D & EndValue, float Duration)
{
    return FTweenManager::Singleton.CreateTween()->To(StartValue, EndValue, Duration);
}

FGTweener* FGTween::To(const FVector& StartValue, const FVector & EndValue, float Duration)
{
    return FTweenManager::Singleton.CreateTween()->To(StartValue, EndValue, Duration);
}

FGTweener* FGTween::To(const FVector4& StartValue, const FVector4 & EndValue, float Duration)
{
    return FTweenManager::Singleton.CreateTween()->To(StartValue, EndValue, Duration);
}

FGTweener* FGTween::To(const FColor& StartValue, const FColor & EndValue, float Duration)
{
    return FTweenManager::Singleton.CreateTween()->To(StartValue, EndValue, Duration);
}

FGTweener* FGTween::ToDouble(double StartValue, double EndValue, float Duration)
{
    return FTweenManager::Singleton.CreateTween()->To(StartValue, EndValue, Duration);
}

FGTweener* FGTween::DelayedCall(float Delay)
{
    return FTweenManager::Singleton.CreateTween()->SetDelay(Delay);
}

FGTweener* FGTween::Shake(const FVector2D& StartValue, float Amplitude, float Duration)
{
    return FTweenManager::Singleton.CreateTween()->Shake(StartValue, Amplitude, Duration);
}

bool FGTween::IsTweening(const FTweenerHandle& Handle)
{
    return FTweenManager::Singleton.IsTweening(Handle);
}

bool FGTween::IsTweening(UObject* Target)
{
    return FTweenManager::Singleton.IsTweening(Target);
}

void FGTween::Kill(FTweenerHandle& Handle, bool bSetComplete)
{
    FTweenManager::Singleton.KillTween(Handle, bSetComplete);
}

void FGTween::Kill(UObject* Target, bool bSetComplete)
{
    FTweenManager::Singleton.KillTweens(Target, bSetComplete);
}

FGTweener* FGTween::GetTween(const FTweenerHandle& Handle)
{
    return FTweenManager::Singleton.GetTween(Handle);
}

FGTweener* FGTween::GetTween(UObject * Target)
{
    return FTweenManager::Singleton.GetTween(Target);
}

void FGTweenAction::MoveX(FGTweener* Tweener)
{
    UGObject * target = Cast<UGObject>(Tweener->GetTarget());
    target->SetX(Tweener->Value.X);
}

void FGTweenAction::MoveY(FGTweener* Tweener)
{
    UGObject * target = Cast<UGObject>(Tweener->GetTarget());
    target->SetY(Tweener->Value.X);
}

void FGTweenAction::Move(FGTweener* Tweener)
{
    UGObject * target = Cast<UGObject>(Tweener->GetTarget());
    target->SetPosition(Tweener->Value.GetVec2());
}

void FGTweenAction::SetWidth(FGTweener* Tweener)
{
    UGObject * target = Cast<UGObject>(Tweener->GetTarget());
    target->SetWidth(Tweener->Value.X);
}

void FGTweenAction::SetHeight(FGTweener* Tweener)
{
    UGObject * target = Cast<UGObject>(Tweener->GetTarget());
    target->SetHeight(Tweener->Value.X);
}

void FGTweenAction::SetSize(FGTweener* Tweener)
{
    UGObject * target = Cast<UGObject>(Tweener->GetTarget());
    target->SetSize(Tweener->Value.GetVec2());
}

void FGTweenAction::ScaleX(FGTweener* Tweener)
{
    UGObject * target = Cast<UGObject>(Tweener->GetTarget());
    target->SetScaleX(Tweener->Value.X);
}

void FGTweenAction::ScaleY(FGTweener* Tweener)
{
    UGObject * target = Cast<UGObject>(Tweener->GetTarget());
    target->SetScaleY(Tweener->Value.X);
}

void FGTweenAction::ScaleXY(FGTweener* Tweener)
{
    UGObject * target = Cast<UGObject>(Tweener->GetTarget());
    target->SetScale(Tweener->Value.GetVec2());
}

void FGTweenAction::Rotate(FGTweener* Tweener)
{
    UGObject * target = Cast<UGObject>(Tweener->GetTarget());
    target->SetRotation(Tweener->Value.X);
}

void FGTweenAction::SetAlpha(FGTweener* Tweener)
{
    UGObject * target = Cast<UGObject>(Tweener->GetTarget());
    target->SetAlpha(Tweener->Value.X);
}

void FGTweenAction::SetProgress(FGTweener* Tweener)
{
    UGProgressBar * target = Cast<UGProgressBar>(Tweener->GetTarget());
    target->Update(Tweener->Value.X);
}
