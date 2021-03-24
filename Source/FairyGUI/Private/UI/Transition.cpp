#include "UI/Transition.h"
#include "UI/GComponent.h"
#include "UI/UIPackage.h"
#include "UI/GController.h"
#include "Utils/ByteBuffer.h"
#include "Tween/GPath.h"

const int32 OPTION_IGNORE_DISPLAY_CONTROLLER = 1;
const int32 OPTION_AUTO_STOP_DISABLED = 2;
const int32 OPTION_AUTO_STOP_AT_END = 4;

struct FAniData
{
    int32 Frame;
    bool bPlaying;
    bool bFlag;
};

struct FSoundData
{
    FString URL;
    float Volume;
};

struct FInnerTransData
{
    FString Name;
    int32 PlayTimes;
    UTransition* Instance;
    float StopTime;
};

struct FShakeData
{
    float Amplitude;
    float Duration;
    FVector2D LastOffset;
    FVector2D Offset;
};

struct FTransitionItemData
{
    float f1;
    float f2;
    float f3;
    float f4;
    bool b1;
    bool b2;
    bool b3;

    FTransitionItemData();
    FVector2D GetVec2() const;
    void SetVec2(const FVector2D& value);
    FVector4 GetVec4() const;
    void SetVec4(const FVector4& value);
    FColor GetColor() const;
    void SetColor(const FColor& value);
};

FTransitionItemData::FTransitionItemData()
{
    f1 = f2 = f3 = f4 = 0;
    b1 = b2 = true;
    b3 = false;
}

FVector2D FTransitionItemData::GetVec2() const
{
    return FVector2D(f1, f2);
}

void FTransitionItemData::SetVec2(const FVector2D& value)
{
    f1 = value.X;
    f2 = value.Y;
}

FVector4 FTransitionItemData::GetVec4() const
{
    return FVector4(f1, f2, f3, f4);
}

void FTransitionItemData::SetVec4(const FVector4& value)
{
    f1 = value.X;
    f2 = value.Y;
    f3 = value.Z;
    f4 = value.W;
}

FColor FTransitionItemData::GetColor() const
{
    return FColor(f1*255.f, f2*255.f, f3*255.f, f4*255.f);
}

void FTransitionItemData::SetColor(const FColor& value)
{
    f1 = value.R / 255.f;
    f2 = value.G / 255.f;
    f3 = value.B / 255.f;
    f4 = value.A / 255.f;
}

struct FTweenConfig
{
    float Duration;
    EEaseType EaseType;
    int32 Repeat;
    bool bYoyo;

    FTransitionItemData StartData;
    FTransitionItemData EndData;
    TSharedPtr<FGPath> Path;

    FString EndLabel;
    FSimpleDelegate EndHook;

    FTweenConfig();
};

FTweenConfig::FTweenConfig() :
    EaseType(EEaseType::QuadOut)
{
}

struct FTransitionItem
{
    float Time;
    FString TargetID;
    ETransitionActionType Type;
    TOptional<FTweenConfig> TweenConfig;
    FString Label;
    FSimpleDelegate Hook;

    TOptional<FTransitionItemData> Data;
    TOptional<bool> VisibleData;
    TOptional<FAniData> AniData;
    TOptional<FSoundData> SoundData;
    TOptional<FInnerTransData> TransData;
    TOptional<FShakeData> ShakeData;
    TOptional<FString> TextData;

    //running properties
    FGTweener* Tweener;
    UGObject* Target;
    uint32 DisplayLockToken;

    FTransitionItem(ETransitionActionType aType);
    ~FTransitionItem();
};

FTransitionItem::FTransitionItem(ETransitionActionType InType) :
    Time(0),
    Type(InType),
    Tweener(nullptr),
    Target(nullptr),
    DisplayLockToken(0)
{
    switch (InType)
    {
    case ETransitionActionType::XY:
    case ETransitionActionType::Size:
    case ETransitionActionType::Scale:
    case ETransitionActionType::Pivot:
    case ETransitionActionType::Skew:
    case ETransitionActionType::Alpha:
    case ETransitionActionType::Rotation:
    case ETransitionActionType::Color:
    case ETransitionActionType::ColorFilter:
        Data.Emplace();
        break;

    case ETransitionActionType::Animation:
        AniData.Emplace();
        break;

    case ETransitionActionType::Shake:
        ShakeData.Emplace();
        break;

    case ETransitionActionType::Sound:
        SoundData.Emplace();
        break;

    case ETransitionActionType::Transition:
        TransData.Emplace();
        break;

    case ETransitionActionType::Visible:
        VisibleData.Emplace();
        break;

    case ETransitionActionType::Text:
    case ETransitionActionType::Icon:
        TextData.Emplace();
        break;

    default:
        break;
    }
}

FTransitionItem::~FTransitionItem()
{
    if (Tweener != nullptr)
        Tweener->Kill();
}

UTransition::UTransition() :
    TotalTimes(0),
    TotalTasks(0),
    bPlaying(false),
    bPaused(false),
    Options(0),
    bReversed(false),
    TotalDuration(0),
    bAutoPlay(false),
    AutoPlayDelay(0),
    TimeScale(1),
    StartTime(0),
    EndTime(0)
{

}

UTransition::~UTransition()
{
    if (DelayHandle.IsValid())
        FGTween::Kill(DelayHandle);

    for (auto &it : Items)
        delete it;
}

void UTransition::Play(int32 InTimes, float InDelay, float InStartTime, float InEndTime, bool bInReverse, FSimpleDelegate InCompleteCallback)
{
    Stop(true, true);

    TotalTimes = InTimes;
    bReversed = bInReverse;
    StartTime = InStartTime;
    EndTime = InEndTime;
    bPlaying = true;
    bPaused = false;
    CompleteCallback = InCompleteCallback;

    int32 cnt = Items.Num();
    for (int32 i = 0; i < cnt; i++)
    {
        FTransitionItem* item = Items[i];
        if (item->Target == nullptr)
        {
            if (!item->TargetID.IsEmpty())
                item->Target = Owner->GetChildByID(item->TargetID);
            else
                item->Target = Owner;
        }
        else if (item->Target != Owner && item->Target->GetParent() != Owner) //maybe removed
            item->Target = nullptr;

        if (item->Target != nullptr && item->Type == ETransitionActionType::Transition)
        {
            UTransition* trans = Cast<UGComponent>(item->Target)->GetTransition(item->TransData->Name);
            if (trans == this)
                trans = nullptr;
            if (trans != nullptr)
            {
                if (item->TransData->PlayTimes == 0) //stop
                {
                    int32 j;
                    for (j = i - 1; j >= 0; j--)
                    {
                        FTransitionItem* item2 = Items[j];
                        if (item2->Type == ETransitionActionType::Transition)
                        {
                            if (item2->TransData->Instance == trans)
                            {
                                item2->TransData->StopTime = item->Time - item2->Time;
                                break;
                            }
                        }
                    }
                    if (j < 0)
                        item->TransData->StopTime = 0;
                    else
                        trans = nullptr; //no need to handle stop anymore
                }
                else
                    item->TransData->StopTime = -1;
            }
            item->TransData->Instance = trans;
        }
    }

    if (InDelay == 0)
        OnDelayedPlay();
    else
        DelayHandle = FGTween::DelayedCall(InDelay)->OnComplete(FSimpleDelegate::CreateUObject(this, &UTransition::OnDelayedPlay))->GetHandle();
}

void UTransition::ChangePlayTimes(int32 InTimes)
{
    TotalTimes = InTimes;
}

void UTransition::SetAutoPlay(bool bInAutoPlay, int32 InTimes, float InDelay)
{
    if (bAutoPlay != bInAutoPlay)
    {
        bAutoPlay = bInAutoPlay;
        AutoPlayTimes = InTimes;
        AutoPlayDelay = InDelay;
        if (bAutoPlay)
        {
            if (Owner->OnStage())
                Play(AutoPlayTimes, AutoPlayDelay);
        }
        else
        {
            if (!Owner->OnStage())
                Stop(false, true);
        }
    }
}

void UTransition::Stop(bool bSetToComplete, bool bProcessCallback)
{
    if (!bPlaying)
        return;

    bPlaying = false;
    TotalTasks = 0;
    TotalTimes = 0;
    FSimpleDelegate func = CompleteCallback;
    CompleteCallback.Unbind();

    int32 cnt = Items.Num();
    if (bReversed)
    {
        for (int32 i = cnt - 1; i >= 0; i--)
        {
            FTransitionItem* item = Items[i];
            if (item->Target == nullptr)
                continue;

            StopItem(item, bSetToComplete);
        }
    }
    else
    {
        for (int32 i = 0; i < cnt; i++)
        {
            FTransitionItem* item = Items[i];
            if (item->Target == nullptr)
                continue;

            StopItem(item, bSetToComplete);
        }
    }
    if (bProcessCallback)
        func.ExecuteIfBound();
}

void UTransition::StopItem(FTransitionItem* item, bool bSetToComplete)
{
    if (item->DisplayLockToken != 0)
    {
        item->Target->ReleaseDisplayLock(item->DisplayLockToken);
        item->DisplayLockToken = 0;
    }

    if (item->Tweener != nullptr)
    {
        item->Tweener->Kill(bSetToComplete);
        item->Tweener = nullptr;

        if (item->Type == ETransitionActionType::Shake && !bSetToComplete)
        {
            item->Target->bGearLocked = true;
            item->Target->SetPosition(item->Target->GetPosition() - item->ShakeData->LastOffset);
            item->Target->bGearLocked = false;
        }
    }
}

void UTransition::SetPaused(bool bInPaused)
{
    if (!bPlaying || bPaused == bInPaused)
        return;

    bPaused = bInPaused;
    FGTweener* tweener = FGTween::GetTween(DelayHandle);
    if (tweener != nullptr)
        tweener->SetPaused(bPaused);

    for (auto& item : Items)
    {
        if (item->Target == nullptr)
            continue;

        if (item->Type == ETransitionActionType::Transition)
        {
            if (item->TransData->Instance != nullptr)
                item->TransData->Instance->SetPaused(bPaused);
        }
        else if (item->Type == ETransitionActionType::Animation)
        {
            if (bPaused)
            {
                item->AniData->bFlag = item->Target->GetProp<bool>(EObjectPropID::Playing);
                item->Target->SetProp(EObjectPropID::Playing, FNVariant(false));
            }
            else
                item->Target->SetProp(EObjectPropID::Playing, FNVariant(item->AniData->bFlag));
        }

        if (item->Tweener != nullptr)
            item->Tweener->SetPaused(bPaused);
    }
}

void UTransition::SetValue(const FString& InLabel, const TArray<FNVariant>& InValues)
{
    FTransitionItemData* Value = nullptr;

    for (auto& item : Items)
    {
        if (item->Label == InLabel)
        {
            if (item->TweenConfig.IsSet())
                Value = &item->TweenConfig->StartData;
            else if (item->Data.IsSet())
                Value = &item->Data.GetValue();
        }
        else if (item->TweenConfig.IsSet() && item->TweenConfig->EndLabel == InLabel)
        {
            Value = &item->TweenConfig->EndData;
        }
        else
            continue;

        switch (item->Type)
        {
        case ETransitionActionType::XY:
        case ETransitionActionType::Size:
        case ETransitionActionType::Pivot:
        case ETransitionActionType::Scale:
        case ETransitionActionType::Skew:
        {
            Value->b1 = true;
            Value->b2 = true;
            Value->f1 = InValues[0].AsFloat();
            Value->f2 = InValues[1].AsFloat();
            break;
        }

        case ETransitionActionType::Alpha:
        case ETransitionActionType::Rotation:
            Value->f1 = InValues[0].AsFloat();
            break;

        case ETransitionActionType::Color:
        {
            Value->SetColor(InValues[0].AsColor());
            break;
        }

        case ETransitionActionType::Animation:
        {
            item->AniData->Frame = InValues[0].AsInt();
            if (InValues.Num() > 1)
                item->AniData->bPlaying = InValues[0].AsBool();
            break;
        }

        case ETransitionActionType::Visible:
            item->VisibleData = InValues[0].AsBool();
            break;

        case ETransitionActionType::Sound:
        {
            item->SoundData->URL = InValues[0].AsString();
            if (InValues.Num() > 1)
                item->SoundData->Volume = InValues[1].AsFloat();
            break;
        }

        case ETransitionActionType::Transition:
        {
            item->TransData->Name = InValues[0].AsString();
            if (InValues.Num() > 1)
                item->TransData->PlayTimes = InValues[1].AsInt();
            break;
        }

        case ETransitionActionType::Shake:
        {
            item->ShakeData->Amplitude = InValues[0].AsFloat();
            if (InValues.Num() > 1)
                item->ShakeData->Duration = InValues[1].AsFloat();
            break;
        }

        case ETransitionActionType::ColorFilter:
        {
            Value->f1 = InValues[0].AsFloat();
            Value->f2 = InValues[1].AsFloat();
            Value->f3 = InValues[2].AsFloat();
            Value->f4 = InValues[3].AsFloat();
            break;
        }

        case ETransitionActionType::Text:
        case ETransitionActionType::Icon:
            item->TextData = InValues[0].AsString();
            break;
        default:
            break;
        }
    }
}

void UTransition::SetHook(const FString& InLabel, FSimpleDelegate Callback)
{
    for (auto& item : Items)
    {
        if (item->Label == InLabel)
        {
            item->Hook = Callback;
            break;
        }
        else if (item->TweenConfig.IsSet() && item->TweenConfig->EndLabel == InLabel)
        {
            item->TweenConfig->EndHook = Callback;
            break;
        }
    }
}

void UTransition::ClearHooks()
{
    for (auto& item : Items)
    {
        item->Hook.Unbind();
        if (item->TweenConfig.IsSet())
            item->TweenConfig->EndHook.Unbind();
    }
}

void UTransition::SetTarget(const FString& InLabel, UGObject* InTarget)
{
    for (auto& item : Items)
    {
        if (item->Label == InLabel)
        {

            item->TargetID = InTarget->ID;
            item->Target = nullptr;
        }
    }
}

void UTransition::SetDuration(const FString& InLabel, float InDuration)
{
    for (auto& item : Items)
    {
        if (item->TweenConfig.IsSet() && item->Label == InLabel)
            item->TweenConfig->Duration = InDuration;
    }
}

float UTransition::GetLabelTime(const FString& InLabel) const
{
    for (auto& item : Items)
    {
        if (item->Label == InLabel)
        {
            if (item->TweenConfig.IsSet())
                return item->Time + item->TweenConfig->Duration;
            else
                return item->Time;
        }
    }

    return NAN;
}

void UTransition::SetTimeScale(float InTimeScale)
{
    if (TimeScale != InTimeScale)
    {
        TimeScale = InTimeScale;

        for (auto& item : Items)
        {
            if (item->Tweener != nullptr)
                item->Tweener->SetTimeScale(InTimeScale);
            else if (item->Type == ETransitionActionType::Transition)
            {
                if (item->TransData->Instance != nullptr)
                    item->TransData->Instance->SetTimeScale(InTimeScale);
            }
            else if (item->Type == ETransitionActionType::Animation)
            {
                if (item->Target != nullptr)
                    item->Target->SetProp(EObjectPropID::TimeScale, FNVariant(InTimeScale));
            }
        }
    }
}

void UTransition::UpdateFromRelations(const FString& TargetID, const FVector2D& Delta)
{
    int32 cnt = Items.Num();
    if (cnt == 0)
        return;

    for (auto& item : Items)
    {
        if (item->Type == ETransitionActionType::XY && item->TargetID == TargetID)
        {
            if (item->TweenConfig.IsSet())
            {
                if (!item->TweenConfig->StartData.b3) {
                    item->TweenConfig->StartData.f1 += Delta.X;
                    item->TweenConfig->StartData.f2 += Delta.Y;
                    item->TweenConfig->EndData.f1 += Delta.X;
                    item->TweenConfig->EndData.f2 += Delta.Y;
                }
            }
            else
            {
                if (!item->Data->b3) {
                    item->Data->f1 += Delta.X;
                    item->Data->f2 += Delta.Y;
                }
            }
        }
    }
}

void UTransition::OnOwnerAddedToStage()
{
    if (bAutoPlay && !bPlaying)
        Play(AutoPlayTimes, AutoPlayDelay);
}

void UTransition::OnOwnerRemovedFromStage()
{
    if ((Options & OPTION_AUTO_STOP_DISABLED) == 0)
        Stop((Options & OPTION_AUTO_STOP_AT_END) != 0 ? true : false, false);
}

void UTransition::OnDelayedPlay()
{
    InternalPlay();

    bPlaying = TotalTasks > 0;
    if (bPlaying)
    {
        if ((Options & OPTION_IGNORE_DISPLAY_CONTROLLER) != 0)
        {
            for (auto& item : Items)
            {
                if (item->Target != nullptr && item->Target != Owner)
                    item->DisplayLockToken = item->Target->AddDisplayLock();
            }
        }
    }
    else if (CompleteCallback.IsBound())
    {
        FSimpleDelegate func = CompleteCallback;
        CompleteCallback.Unbind();
        func.Execute();
    }
}

void UTransition::InternalPlay()
{
    OwnerBasePos = Owner->GetPosition();

    TotalTasks = 0;

    bool bNeedSkipAnimations = false;
    int32 cnt = Items.Num();
    if (!bReversed)
    {
        for (int32 i = 0; i < cnt; i++)
        {
            FTransitionItem* item = Items[i];
            if (item->Target == nullptr)
                continue;

            if (item->Type == ETransitionActionType::Animation && StartTime != 0 && item->Time <= StartTime)
            {
                bNeedSkipAnimations = true;
                item->AniData->bFlag = false;
            }
            else
                PlayItem(item);
        }
    }
    else
    {
        for (int32 i = cnt - 1; i >= 0; i--)
        {
            FTransitionItem* item = Items[i];
            if (item->Target == nullptr)
                continue;

            PlayItem(item);
        }
    }

    if (bNeedSkipAnimations)
        SkipAnimations();
}

void UTransition::PlayItem(FTransitionItem* item)
{
    float time;
    if (item->TweenConfig.IsSet())
    {
        if (bReversed)
            time = (TotalDuration - item->Time - item->TweenConfig->Duration);
        else
            time = item->Time;

        if (EndTime == -1 || time <= EndTime)
        {
            FTransitionItemData* startValue;
            FTransitionItemData* endValue;

            if (bReversed)
            {
                startValue = &item->TweenConfig->EndData;
                endValue = &item->TweenConfig->StartData;
            }
            else
            {
                startValue = &item->TweenConfig->StartData;
                endValue = &item->TweenConfig->EndData;
            }

            item->Data->b1 = startValue->b1 || endValue->b1;
            item->Data->b2 = startValue->b2 || endValue->b2;

            switch (item->Type)
            {
            case ETransitionActionType::XY:
            case ETransitionActionType::Size:
            case ETransitionActionType::Scale:
            case ETransitionActionType::Skew:
                item->Tweener = FGTween::To(startValue->GetVec2(), endValue->GetVec2(), item->TweenConfig->Duration);
                break;

            case ETransitionActionType::Alpha:
            case ETransitionActionType::Rotation:
                item->Tweener = FGTween::To(startValue->f1, endValue->f1, item->TweenConfig->Duration);
                break;

            case ETransitionActionType::Color:
                item->Tweener = FGTween::To(startValue->GetColor(), endValue->GetColor(), item->TweenConfig->Duration);
                break;

            case ETransitionActionType::ColorFilter:
                item->Tweener = FGTween::To(startValue->GetVec4(), endValue->GetVec4(), item->TweenConfig->Duration);
                break;
            default:
                break;
            }

            item->Tweener->SetDelay(time)
                ->SetEase(item->TweenConfig->EaseType)
                ->SetRepeat(item->TweenConfig->Repeat, item->TweenConfig->bYoyo)
                ->SetTimeScale(TimeScale)
                ->SetUserData(FNVariant(item))
                ->OnStart(FTweenDelegate::CreateUObject(this, &UTransition::OnTweenStart))
                ->OnUpdate(FTweenDelegate::CreateUObject(this, &UTransition::OnTweenUpdate))
                ->OnComplete(FTweenDelegate::CreateUObject(this, &UTransition::OnTweenComplete));

            if (EndTime >= 0)
                item->Tweener->SetBreakpoint(EndTime - time);

            TotalTasks++;
        }
    }
    else if (item->Type == ETransitionActionType::Shake)
    {
        if (bReversed)
            time = (TotalDuration - item->Time - item->ShakeData->Duration);
        else
            time = item->Time;

        if (EndTime == -1 || time <= EndTime)
        {
            item->ShakeData->LastOffset.Set(0, 0);
            item->ShakeData->Offset.Set(0, 0);
            item->Tweener = FGTween::Shake(FVector2D::ZeroVector, item->ShakeData->Amplitude, item->ShakeData->Duration)
                ->SetDelay(time)
                ->SetTimeScale(TimeScale)
                ->SetUserData(FNVariant(item))
                ->OnStart(FTweenDelegate::CreateUObject(this, &UTransition::OnTweenStart))
                ->OnUpdate(FTweenDelegate::CreateUObject(this, &UTransition::OnTweenUpdate))
                ->OnComplete(FTweenDelegate::CreateUObject(this, &UTransition::OnTweenComplete));

            if (EndTime >= 0)
                item->Tweener->SetBreakpoint(EndTime - item->Time);

            TotalTasks++;
        }
    }
    else
    {
        if (bReversed)
            time = (TotalDuration - item->Time);
        else
            time = item->Time;

        if (time <= StartTime)
        {
            ApplyValue(item);
            CallHook(item, false);
        }
        else if (EndTime == -1 || time <= EndTime)
        {
            TotalTasks++;
            item->Tweener = FGTween::DelayedCall(time)
                ->SetTimeScale(TimeScale)
                ->SetUserData(FNVariant(item))
                ->OnComplete(FTweenDelegate::CreateUObject(this, &UTransition::OnDelayedPlayItem));
        }
    }

    if (item->Tweener != nullptr)
        item->Tweener->Seek(StartTime);
}

void UTransition::SkipAnimations()
{
    int32 frame;
    float playStartTime;
    float playTotalTime;
    UGObject* target;

    int32 cnt = Items.Num();
    for (int32 i = 0; i < cnt; i++)
    {
        FTransitionItem* item = Items[i];
        if (item->Type != ETransitionActionType::Animation || item->Time > StartTime)
            continue;

        if (item->AniData->bFlag)
            continue;

        target = item->Target;
        frame = target->GetProp<int32>(EObjectPropID::Frame);
        playStartTime = target->GetProp<bool>(EObjectPropID::Playing) ? 0 : -1;
        playTotalTime = 0;

        for (int32 j = i; j < cnt; j++)
        {
            item = Items[j];
            if (item->Type != ETransitionActionType::Animation || item->Target != target || item->Time > StartTime)
                continue;

            item->AniData->bFlag = true;

            if (item->AniData->Frame != -1)
            {
                frame = item->AniData->Frame;
                if (item->AniData->bPlaying)
                    playStartTime = item->Time;
                else
                    playStartTime = -1;
                playTotalTime = 0;
            }
            else
            {
                if (item->AniData->bPlaying)
                {
                    if (playStartTime < 0)
                        playStartTime = item->Time;
                }
                else
                {
                    if (playStartTime >= 0)
                        playTotalTime += (item->Time - playStartTime);
                    playStartTime = -1;
                }
            }

            CallHook(item, false);
        }

        if (playStartTime >= 0)
            playTotalTime += (StartTime - playStartTime);

        target->SetProp(EObjectPropID::Playing, FNVariant(playStartTime >= 0));
        target->SetProp(EObjectPropID::Frame, FNVariant(frame));
        if (playTotalTime > 0)
            target->SetProp(EObjectPropID::DeltaTime, FNVariant(playTotalTime));
    }
}

void UTransition::OnDelayedPlayItem(FGTweener* Tweener)
{
    FTransitionItem* item = (FTransitionItem*)Tweener->GetUserData().As<void*>();
    item->Tweener = nullptr;
    TotalTasks--;

    ApplyValue(item);
    CallHook(item, false);

    CheckAllComplete();
}

void UTransition::OnTweenStart(FGTweener* Tweener)
{
    FTransitionItem* item = (FTransitionItem*)Tweener->GetUserData().As<void*>();

    if (item->Type == ETransitionActionType::XY || item->Type == ETransitionActionType::Size)
    {
        FTransitionItemData* startValue;
        FTransitionItemData* endValue;

        if (bReversed)
        {
            startValue = &item->TweenConfig->EndData;
            endValue = &item->TweenConfig->StartData;
        }
        else
        {
            startValue = &item->TweenConfig->StartData;
            endValue = &item->TweenConfig->EndData;
        }

        if (item->Type == ETransitionActionType::XY)
        {
            if (item->Target != Owner)
            {
                if (!startValue->b1)
                    Tweener->StartValue.X = item->Target->GetX();
                else if (startValue->b3) //percent
                    Tweener->StartValue.X = startValue->f1 * Owner->GetWidth();

                if (!startValue->b2)
                    Tweener->StartValue.Y = item->Target->GetY();
                else if (startValue->b3) //percent
                    Tweener->StartValue.Y = startValue->f2 * Owner->GetHeight();

                if (!endValue->b1)
                    Tweener->EndValue.X = Tweener->StartValue.X;
                else if (endValue->b3)
                    Tweener->EndValue.X = endValue->f1 * Owner->GetWidth();

                if (!endValue->b2)
                    Tweener->EndValue.Y = Tweener->StartValue.Y;
                else if (endValue->b3)
                    Tweener->EndValue.Y = endValue->f2 * Owner->GetHeight();
            }
            else
            {
                if (!startValue->b1)
                    Tweener->StartValue.X = item->Target->GetX() - OwnerBasePos.X;
                if (!startValue->b2)
                    Tweener->StartValue.Y = item->Target->GetY() - OwnerBasePos.Y;

                if (!endValue->b1)
                    Tweener->EndValue.X = Tweener->StartValue.X;
                if (!endValue->b2)
                    Tweener->EndValue.Y = Tweener->StartValue.Y;
            }
        }
        else
        {
            if (!startValue->b1)
                Tweener->StartValue.X = item->Target->GetWidth();
            if (!startValue->b2)
                Tweener->StartValue.Y = item->Target->GetHeight();

            if (!endValue->b1)
                Tweener->EndValue.X = Tweener->StartValue.X;
            if (!endValue->b2)
                Tweener->EndValue.Y = Tweener->StartValue.Y;
        }

        if (item->TweenConfig->Path.IsValid())
        {
            item->Data->b1 = item->Data->b2 = true;
            Tweener->SetPath(item->TweenConfig->Path);
        }
    }

    CallHook(item, false);
}

void UTransition::OnTweenUpdate(FGTweener* Tweener)
{
    FTransitionItem* item = (FTransitionItem*)Tweener->GetUserData().As<void*>();

    switch (item->Type)
    {
    case ETransitionActionType::XY:
    case ETransitionActionType::Size:
    case ETransitionActionType::Scale:
    case ETransitionActionType::Skew:
        if (item->TweenConfig->Path.IsValid())
            item->Data->SetVec2(Tweener->Value.GetVec2() + Tweener->StartValue.GetVec2());
        else
            item->Data->SetVec2(Tweener->Value.GetVec2());
        break;

    case ETransitionActionType::Alpha:
    case ETransitionActionType::Rotation:
        item->Data->f1 = Tweener->Value.X;
        break;

    case ETransitionActionType::Color:
        item->Data->SetColor(Tweener->Value.GetColor());
        break;

    case ETransitionActionType::ColorFilter:
        item->Data->SetVec4(Tweener->Value.GetVec4());
        break;

    case ETransitionActionType::Shake:
        item->ShakeData->Offset = Tweener->DeltaValue.GetVec2();
        break;
    default:
        break;
    }
    ApplyValue(item);
}

void UTransition::OnTweenComplete(FGTweener* Tweener)
{
    FTransitionItem* item = (FTransitionItem*)Tweener->GetUserData().As<void*>();
    item->Tweener = nullptr;
    TotalTasks--;

    if (Tweener->AllCompleted())
        CallHook(item, true);

    CheckAllComplete();
}

void UTransition::OnPlayTransCompleted(FTransitionItem* item)
{
    TotalTasks--;

    CheckAllComplete();
}

void UTransition::CallHook(FTransitionItem* item, bool bTweenEnd)
{
    if (bTweenEnd)
    {
        if (item->TweenConfig.IsSet() && item->TweenConfig->EndHook.IsBound())
            item->TweenConfig->EndHook.Execute();
    }
    else
    {
        if (item->Time >= StartTime && item->Hook.IsBound())
            item->Hook.Execute();
    }
}

void UTransition::CheckAllComplete()
{
    if (bPlaying && TotalTasks == 0)
    {
        if (TotalTimes < 0)
        {
            InternalPlay();
        }
        else
        {
            TotalTimes--;
            if (TotalTimes > 0)
                InternalPlay();
            else
            {
                bPlaying = false;

                for (auto& item : Items)
                {
                    if (item->Target != nullptr && item->DisplayLockToken != 0)
                    {
                        item->Target->ReleaseDisplayLock(item->DisplayLockToken);
                        item->DisplayLockToken = 0;
                    }
                }

                if (CompleteCallback.IsBound())
                {
                    FSimpleDelegate func = CompleteCallback;
                    CompleteCallback.Unbind();
                    func.Execute();
                }
            }
        }
    }
}

void UTransition::ApplyValue(FTransitionItem* item)
{
    item->Target->bGearLocked = true;

    switch (item->Type)
    {
    case ETransitionActionType::XY:
    {
        if (item->Target == Owner)
        {
            if (item->Data->b1 && item->Data->b2)
                item->Target->SetPosition(item->Data->GetVec2() + OwnerBasePos);
            else if (item->Data->b1)
                item->Target->SetX(item->Data->f1 + OwnerBasePos.X);
            else
                item->Target->SetY(item->Data->f2 + OwnerBasePos.Y);
        }
        else
        {
            if (item->Data->b3) //position in percent
            {
                if (item->Data->b1 && item->Data->b2)
                    item->Target->SetPosition(item->Data->GetVec2() * Owner->GetSize());
                else if (item->Data->b1)
                    item->Target->SetX(item->Data->f1 * Owner->GetWidth());
                else if (item->Data->b2)
                    item->Target->SetY(item->Data->f2 * Owner->GetHeight());
            }
            else
            {
                if (item->Data->b1 && item->Data->b2)
                    item->Target->SetPosition(item->Data->GetVec2());
                else if (item->Data->b1)
                    item->Target->SetX(item->Data->f1);
                else if (item->Data->b2)
                    item->Target->SetY(item->Data->f2);
            }
        }
    }
    break;

    case ETransitionActionType::Size:
    {
        if (!item->Data->b1)
            item->Data->f1 = item->Target->GetWidth();
        if (!item->Data->b2)
            item->Data->f2 = item->Target->GetHeight();
        item->Target->SetSize(item->Data->GetVec2());
    }
    break;

    case ETransitionActionType::Pivot:
        item->Target->SetPivot(item->Data->GetVec2(), item->Target->IsPivotAsAnchor());
        break;

    case ETransitionActionType::Alpha:
        item->Target->SetAlpha(item->Data->f1);
        break;

    case ETransitionActionType::Rotation:
        item->Target->SetRotation(item->Data->f1);
        break;

    case ETransitionActionType::Scale:
        item->Target->SetScale(item->Data->GetVec2());
        break;

    case ETransitionActionType::Skew:
        item->Target->SetSkew(item->Data->GetVec2());
        break;

    case ETransitionActionType::Color:
        item->Target->SetProp(EObjectPropID::Color, FNVariant(item->Data->GetColor()));
        break;

    case ETransitionActionType::Animation:
    {
        if (item->AniData->Frame >= 0)
            item->Target->SetProp(EObjectPropID::Frame, FNVariant(item->AniData->Frame));
        item->Target->SetProp(EObjectPropID::Playing, FNVariant(item->AniData->bPlaying));
        item->Target->SetProp(EObjectPropID::TimeScale, FNVariant(TimeScale));
        break;
    }

    case ETransitionActionType::Visible:
        item->Target->SetVisible(item->VisibleData.GetValue());
        break;

    case ETransitionActionType::Shake:
    {
        item->Target->SetPosition(item->Target->GetPosition() - item->ShakeData->LastOffset + item->ShakeData->Offset);
        item->ShakeData->LastOffset = item->ShakeData->Offset;
        break;
    }

    case ETransitionActionType::Transition:
        if (bPlaying)
        {
            if (item->TransData->Instance != nullptr)
            {
                TotalTasks++;

                float playStartTime = StartTime > item->Time ? (StartTime - item->Time) : 0;
                float playEndTime = EndTime >= 0 ? (EndTime - item->Time) : -1;
                if (item->TransData->StopTime >= 0 && (playEndTime < 0 || playEndTime > item->TransData->StopTime))
                    playEndTime = item->TransData->StopTime;
                item->TransData->Instance->SetTimeScale(TimeScale);
                item->TransData->Instance->Play(item->TransData->PlayTimes, 0, playStartTime, playEndTime, bReversed,
                    FSimpleDelegate::CreateUObject(this, &UTransition::OnPlayTransCompleted, item));
            }
        }
        break;

    case ETransitionActionType::Sound:
        if (bPlaying && item->Time >= StartTime)
        {
            if (!item->SoundData->URL.IsEmpty())
                Owner->GetApp()->PlaySound(item->SoundData->URL, item->SoundData->Volume);
            break;
        }

    case ETransitionActionType::ColorFilter:
        break;

    case ETransitionActionType::Text:
        item->Target->SetText(item->TextData.GetValue());
        break;

    case ETransitionActionType::Icon:
        item->Target->SetIcon(item->TextData.GetValue());
        break;
    default:
        break;
    }

    item->Target->bGearLocked = false;
}

void UTransition::Setup(FByteBuffer* Buffer)
{
    Owner = Cast<UGComponent>(GetOuter());

    Name = Buffer->ReadS();
    Options = Buffer->ReadInt();
    bAutoPlay = Buffer->ReadBool();
    AutoPlayTimes = Buffer->ReadInt();
    AutoPlayDelay = Buffer->ReadFloat();

    int32 cnt = Buffer->ReadShort();
    for (int32 i = 0; i < cnt; i++)
    {
        int32 dataLen = Buffer->ReadShort();
        int32 curPos = Buffer->GetPos();

        Buffer->Seek(curPos, 0);

        Items.Add(new FTransitionItem((ETransitionActionType)Buffer->ReadByte()));
        FTransitionItem* item = Items.Last();

        item->Time = Buffer->ReadFloat();
        int32 TargetID = Buffer->ReadShort();
        if (TargetID < 0)
            item->TargetID = G_EMPTY_STRING;
        else
            item->TargetID = Owner->GetChildAt(TargetID)->ID;
        item->Label = Buffer->ReadS();

        if (Buffer->ReadBool())
        {
            Buffer->Seek(curPos, 1);

            item->TweenConfig.Emplace();
            item->TweenConfig->Duration = Buffer->ReadFloat();
            if (item->Time + item->TweenConfig->Duration > TotalDuration)
                TotalDuration = item->Time + item->TweenConfig->Duration;
            item->TweenConfig->EaseType = (EEaseType)Buffer->ReadByte();
            item->TweenConfig->Repeat = Buffer->ReadInt();
            item->TweenConfig->bYoyo = Buffer->ReadBool();
            item->TweenConfig->EndLabel = Buffer->ReadS();

            Buffer->Seek(curPos, 2);

            DecodeValue(item, Buffer, &item->TweenConfig->StartData);

            Buffer->Seek(curPos, 3);

            DecodeValue(item, Buffer, &item->TweenConfig->EndData);

            if (Buffer->Version >= 2)
            {
                int32 pathLen = Buffer->ReadInt();
                if (pathLen > 0)
                {
                    item->TweenConfig->Path = MakeShareable(new FGPath());
                    TArray<FGPathPoint> pts;

                    FVector v0(ForceInit), v1(ForceInit), v2(ForceInit);

                    for (int32 j = 0; j < pathLen; j++)
                    {
                        FGPathPoint::ECurveType curveType = (FGPathPoint::ECurveType)Buffer->ReadByte();
                        switch (curveType)
                        {
                        case FGPathPoint::ECurveType::Bezier:
                            v0.X = Buffer->ReadFloat();
                            v0.Y = Buffer->ReadFloat();
                            v1.X = Buffer->ReadFloat();
                            v1.Y = Buffer->ReadFloat();
                            pts.Add(FGPathPoint(v0, v1));
                            break;

                        case FGPathPoint::ECurveType::CubicBezier:
                            v0.X = Buffer->ReadFloat();
                            v0.Y = Buffer->ReadFloat();
                            v1.X = Buffer->ReadFloat();
                            v1.Y = Buffer->ReadFloat();
                            v2.X = Buffer->ReadFloat();
                            v2.Y = Buffer->ReadFloat();
                            pts.Add(FGPathPoint(v0, v1, v2));
                            break;

                        default:
                            v0.X = Buffer->ReadFloat();
                            v0.Y = Buffer->ReadFloat();
                            pts.Add(FGPathPoint(v0, curveType));
                            break;
                        }
                    }

                    item->TweenConfig->Path->Create(pts.GetData(), pts.Num());
                }
            }
        }
        else
        {
            if (item->Time > TotalDuration)
                TotalDuration = item->Time;

            Buffer->Seek(curPos, 2);

            DecodeValue(item, Buffer, item->Data.IsSet() ? &item->Data.GetValue() : nullptr);
        }

        Buffer->SetPos(curPos + dataLen);
    }
}

void UTransition::DecodeValue(FTransitionItem* item, FByteBuffer* Buffer, FTransitionItemData* Value)
{
    switch (item->Type)
    {
    case ETransitionActionType::XY:
    case ETransitionActionType::Size:
    case ETransitionActionType::Pivot:
    case ETransitionActionType::Skew:
    {
        Value->b1 = Buffer->ReadBool();
        Value->b2 = Buffer->ReadBool();
        Value->f1 = Buffer->ReadFloat();
        Value->f2 = Buffer->ReadFloat();

        if (Buffer->Version >= 2 && item->Type == ETransitionActionType::XY)
            Value->b3 = Buffer->ReadBool(); //percent
        break;
    }

    case ETransitionActionType::Alpha:
    case ETransitionActionType::Rotation:
        Value->f1 = Buffer->ReadFloat();
        break;

    case ETransitionActionType::Scale:
    {
        Value->f1 = Buffer->ReadFloat();
        Value->f2 = Buffer->ReadFloat();
        break;
    }

    case ETransitionActionType::Color:
        Value->SetColor(Buffer->ReadColor());
        break;

    case ETransitionActionType::Animation:
    {
        item->AniData->bPlaying = Buffer->ReadBool();
        item->AniData->Frame = Buffer->ReadInt();
        break;
    }

    case ETransitionActionType::Visible:
        item->VisibleData = Buffer->ReadBool();
        break;

    case ETransitionActionType::Sound:
    {
        item->SoundData->URL = Buffer->ReadS();
        item->SoundData->Volume = Buffer->ReadFloat();
        break;
    }

    case ETransitionActionType::Transition:
    {
        item->TransData->Name = Buffer->ReadS();
        item->TransData->PlayTimes = Buffer->ReadInt();
        break;
    }

    case ETransitionActionType::Shake:
    {
        item->ShakeData->Amplitude = Buffer->ReadFloat();
        item->ShakeData->Duration = Buffer->ReadFloat();
        break;
    }

    case ETransitionActionType::ColorFilter:
    {
        Value->f1 = Buffer->ReadFloat();
        Value->f2 = Buffer->ReadFloat();
        Value->f3 = Buffer->ReadFloat();
        Value->f4 = Buffer->ReadFloat();
        break;
    }

    case ETransitionActionType::Text:
    case ETransitionActionType::Icon:
        item->TextData = Buffer->ReadS();
        break;

    default:
        break;
    }
}