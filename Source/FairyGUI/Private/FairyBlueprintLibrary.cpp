#include "FairyBlueprintLibrary.h"
#include "UI/UIConfig.h"
#include "UI/UIObjectFactory.h"
#include "Tween/GTween.h"

const FUIConfig& UFairyBlueprintLibrary::GetUIConfig()
{
    return FUIConfig::Config;
}

void UFairyBlueprintLibrary::SetUIConfig(const FUIConfig& InConfig)
{
    FUIConfig::Config = InConfig;
}

bool UFairyBlueprintLibrary::GetVariantAsBool(UPARAM(ref) FNVariant& InVariant)
{
    return InVariant.AsBool();
}

int32 UFairyBlueprintLibrary::GetVariantAsInt(UPARAM(ref) FNVariant& InVariant)
{
    return InVariant.AsInt();
}

float UFairyBlueprintLibrary::GetVariantAsFloat(UPARAM(ref) FNVariant& InVariant)
{
    return InVariant.AsFloat();
}

FString UFairyBlueprintLibrary::GetVariantAsString(UPARAM(ref) FNVariant& InVariant)
{
    return InVariant.AsString();
}

FColor UFairyBlueprintLibrary::GetVariantAsColor(UPARAM(ref) FNVariant& InVariant)
{
    return InVariant.AsColor();
}

UObject* UFairyBlueprintLibrary::GetVariantAsUObject(UPARAM(ref) FNVariant& InVariant, TSubclassOf<UObject> ClassType)
{
    return InVariant.AsUObject();
}

FNVariant& UFairyBlueprintLibrary::SetVariantBool(UPARAM(ref) FNVariant& InVariant, bool bInValue)
{
    InVariant = bInValue;
    return InVariant;
}

FNVariant& UFairyBlueprintLibrary::SetVariantInt(UPARAM(ref) FNVariant& InVariant, int32 InValue)
{
    InVariant = InValue;
    return InVariant;
}

FNVariant& UFairyBlueprintLibrary::SetVariantFloat(UPARAM(ref) FNVariant& InVariant, float InValue)
{
    InVariant = InValue;
    return InVariant;
}

FNVariant& UFairyBlueprintLibrary::SetVariantString(UPARAM(ref) FNVariant& InVariant, const FString& InValue)
{
    InVariant = InValue;
    return InVariant;
}

FNVariant& UFairyBlueprintLibrary::SetVariantColor(UPARAM(ref) FNVariant& InVariant, const FColor& InValue)
{
    InVariant = InValue;
    return InVariant;
}

FNVariant& UFairyBlueprintLibrary::SetVariantUObject(UPARAM(ref) FNVariant& InVariant, UObject* InValue)
{
    InVariant = (void*)InValue;
    return InVariant;
}

FTweenerHandle UFairyBlueprintLibrary::TweenFloat(float StartValue, float EndValue, EEaseType EaseType, float Duration, int32 Repeat, const FTweenUpdateDynDelegate& OnUpdate, const FSimpleDynDelegate& OnComplete)
{
    const UObject* Target = nullptr;
    if (OnUpdate.IsBound())
        Target = OnUpdate.GetUObject();
    else
        Target = OnComplete.GetUObject();

    if (Target == nullptr)
        return FTweenerHandle();

    FGTweener* Tweener = FGTween::To(StartValue, EndValue, Duration)
        ->SetEase(EaseType)
        ->SetRepeat(Repeat)
        ->SetTarget(const_cast<UObject*>(Target));
    if (OnUpdate.IsBound())
    {
        Tweener->OnUpdate(FTweenDelegate::CreateLambda([OnUpdate](FGTweener* Tweener) {
            OnUpdate.ExecuteIfBound(Tweener->Value, Tweener->DeltaValue);
        }));
    }

    if (OnComplete.IsBound())
    {
        Tweener->OnComplete(FSimpleDelegate::CreateLambda([OnComplete]() {
            OnComplete.ExecuteIfBound();
        }));
    }

    return Tweener->GetHandle();
}

FTweenerHandle UFairyBlueprintLibrary::TweenVector2(const FVector2D& StartValue, const FVector2D& EndValue, EEaseType EaseType, float Duration, int32 Repeat, const FTweenUpdateDynDelegate& OnUpdate, const FSimpleDynDelegate& OnComplete)
{
    const UObject* Target = nullptr;
    if (OnUpdate.IsBound())
        Target = OnUpdate.GetUObject();
    else
        Target = OnComplete.GetUObject();

    if (Target == nullptr)
        return FTweenerHandle();

    FGTweener* Tweener = FGTween::To(StartValue, EndValue, Duration)
        ->SetEase(EaseType)
        ->SetRepeat(Repeat)
        ->SetTarget(const_cast<UObject*>(Target));

    if (OnUpdate.IsBound())
    {
        Tweener->OnUpdate(FTweenDelegate::CreateLambda([OnUpdate](FGTweener* Tweener) {
            OnUpdate.ExecuteIfBound(Tweener->Value, Tweener->DeltaValue);
        }));
    }

    if (OnComplete.IsBound())
    {
        Tweener->OnComplete(FSimpleDelegate::CreateLambda([OnComplete]() {
            OnComplete.ExecuteIfBound();
        }));
    }

    return Tweener->GetHandle();
}

void UFairyBlueprintLibrary::KillTween(UPARAM(ref) FTweenerHandle& Handle, bool bSetComplete)
{
    FGTween::Kill(Handle, bSetComplete);
}

void UFairyBlueprintLibrary::SetPackageItemExtension(const FString& URL, TSubclassOf<UGComponent> ClassType)
{
    FUIObjectFactory::SetExtension(URL, ClassType);
}