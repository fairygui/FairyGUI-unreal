#include "Tween/GTween.h"
#include "Tween/TweenManager.h"
#include "FairyApplication.h"
#include "UI/GProgressBar.h"

FGTweener* FGTween::To(float StartValue, float EndValue, float Duration)
{
    return UFairyApplication::Get()->TweenManager.CreateTween()->To(StartValue, EndValue, Duration);
}

FGTweener* FGTween::To(const FVector2D& StartValue, const FVector2D & EndValue, float Duration)
{
    return UFairyApplication::Get()->TweenManager.CreateTween()->To(StartValue, EndValue, Duration);
}

FGTweener* FGTween::To(const FVector& StartValue, const FVector & EndValue, float Duration)
{
    return UFairyApplication::Get()->TweenManager.CreateTween()->To(StartValue, EndValue, Duration);
}

FGTweener* FGTween::To(const FVector4& StartValue, const FVector4 & EndValue, float Duration)
{
    return UFairyApplication::Get()->TweenManager.CreateTween()->To(StartValue, EndValue, Duration);
}

FGTweener* FGTween::To(const FColor& StartValue, const FColor & EndValue, float Duration)
{
    return UFairyApplication::Get()->TweenManager.CreateTween()->To(StartValue, EndValue, Duration);
}

FGTweener* FGTween::ToDouble(double StartValue, double EndValue, float Duration)
{
    return UFairyApplication::Get()->TweenManager.CreateTween()->To(StartValue, EndValue, Duration);
}

FGTweener* FGTween::DelayedCall(float Delay)
{
    return UFairyApplication::Get()->TweenManager.CreateTween()->SetDelay(Delay);
}

FGTweener* FGTween::Shake(const FVector2D& StartValue, float Amplitude, float Duration)
{
    return UFairyApplication::Get()->TweenManager.CreateTween()->Shake(StartValue, Amplitude, Duration);
}

bool FGTween::IsTweening(const FTweenerHandle& Handle)
{
    if (!UFairyApplication::IsStarted())
        return false;

    return UFairyApplication::Get()->TweenManager.IsTweening(Handle);
}

bool FGTween::IsTweening(UObject* Target)
{
    if (!UFairyApplication::IsStarted())
        return false;

    return UFairyApplication::Get()->TweenManager.IsTweening(Target);
}

void FGTween::Kill(FTweenerHandle& Handle, bool bSetComplete)
{
    if (!UFairyApplication::IsStarted())
        return;

    UFairyApplication::Get()->TweenManager.KillTween(Handle, bSetComplete);
}

void FGTween::Kill(UObject* Target, bool bSetComplete)
{
    if (!UFairyApplication::IsStarted())
        return;

    UFairyApplication::Get()->TweenManager.KillTweens(Target, bSetComplete);
}

FGTweener* FGTween::GetTween(const FTweenerHandle& Handle)
{
    if (!UFairyApplication::IsStarted())
        return nullptr;

    return UFairyApplication::Get()->TweenManager.GetTween(Handle);
}

FGTweener* FGTween::GetTween(UObject * Target)
{
    if (!UFairyApplication::IsStarted())
        return nullptr;

    return UFairyApplication::Get()->TweenManager.GetTween(Target);
}

void FGTween::Action::MoveX(FGTweener* Tweener)
{
    UGObject * target = Cast<UGObject>(Tweener->GetTarget());
    target->SetX(Tweener->Value.X);
}

void FGTween::Action::MoveY(FGTweener* Tweener)
{
    UGObject * target = Cast<UGObject>(Tweener->GetTarget());
    target->SetY(Tweener->Value.X);
}

void FGTween::Action::Move(FGTweener* Tweener)
{
    UGObject * target = Cast<UGObject>(Tweener->GetTarget());
    target->SetPosition(Tweener->Value.GetVec2());
}

void FGTween::Action::SetWidth(FGTweener* Tweener)
{
    UGObject * target = Cast<UGObject>(Tweener->GetTarget());
    target->SetWidth(Tweener->Value.X);
}

void FGTween::Action::SetHeight(FGTweener* Tweener)
{
    UGObject * target = Cast<UGObject>(Tweener->GetTarget());
    target->SetHeight(Tweener->Value.X);
}

void FGTween::Action::SetSize(FGTweener* Tweener)
{
    UGObject * target = Cast<UGObject>(Tweener->GetTarget());
    target->SetSize(Tweener->Value.GetVec2());
}

void FGTween::Action::ScaleX(FGTweener* Tweener)
{
    UGObject * target = Cast<UGObject>(Tweener->GetTarget());
    target->SetScaleX(Tweener->Value.X);
}

void FGTween::Action::ScaleY(FGTweener* Tweener)
{
    UGObject * target = Cast<UGObject>(Tweener->GetTarget());
    target->SetScaleY(Tweener->Value.X);
}

void FGTween::Action::ScaleXY(FGTweener* Tweener)
{
    UGObject * target = Cast<UGObject>(Tweener->GetTarget());
    target->SetScale(Tweener->Value.GetVec2());
}

void FGTween::Action::Rotate(FGTweener* Tweener)
{
    UGObject * target = Cast<UGObject>(Tweener->GetTarget());
    target->SetRotation(Tweener->Value.X);
}

void FGTween::Action::SetAlpha(FGTweener* Tweener)
{
    UGObject * target = Cast<UGObject>(Tweener->GetTarget());
    target->SetAlpha(Tweener->Value.X);
}

void FGTween::Action::SetProgress(FGTweener* Tweener)
{
    UGProgressBar * target = Cast<UGProgressBar>(Tweener->GetTarget());
    target->Update(Tweener->Value.D);
}
