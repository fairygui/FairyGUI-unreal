#include "Tween/TweenManager.h"
#include "Tween/GTweener.h"

FTweenManager FTweenManager::Singleton;

FTweenManager::FTweenManager()
{
    TotalActiveTweens = 0;
    ArrayLength = 30;
    ActiveTweens = new FGTweener*[ArrayLength];
}

FTweenManager::~FTweenManager()
{
    Reset();
    delete []ActiveTweens;
}

void FTweenManager::Reset()
{
    for (auto it : TweenerPool)
        delete it;
    TweenerPool.Reset();

    int32 cnt = TotalActiveTweens;
    for (int32 i = 0; i < cnt; i++)
    {
        FGTweener* tweener = ActiveTweens[i];
        if (tweener != nullptr)
            delete tweener;
    }
    TotalActiveTweens = 0;
}

FGTweener* FTweenManager::CreateTween()
{
    FGTweener* tweener;
    int32 cnt = TweenerPool.Num();
    if (cnt > 0)
    {
        tweener = TweenerPool.Pop();
        tweener->Handle.IncreaseSerialNumber();
    }
    else
    {
        TweenerInstanceCount++;
        if (!ensureMsgf(TweenerInstanceCount != FTweenerHandle::MaxIndex, TEXT("Tweener index number has wrapped around!")))
        {
            TweenerInstanceCount = 0;
        }
        tweener = new FGTweener();
        tweener->Handle.SetIndex(TweenerInstanceCount);
    }
    tweener->Init();
    ActiveTweens[TotalActiveTweens++] = tweener;

    if (TotalActiveTweens == ArrayLength)
    {
        int32 newLen = ArrayLength + FMath::CeilToInt(ArrayLength * 0.5f);
        FGTweener** newArray = new FGTweener*[newLen];
        FMemory::Memcpy(newArray, ActiveTweens, ArrayLength * sizeof(FGTweener*));
        delete []ActiveTweens;
        ActiveTweens = newArray;
        ArrayLength = newLen;
    }

    return tweener;
}

bool FTweenManager::KillTween(FTweenerHandle & Handle, bool bCompleted)
{
    int32 cnt = TotalActiveTweens;
    for (int32 i = 0; i < cnt; i++)
    {
        FGTweener* tweener = ActiveTweens[i];
        if (tweener != nullptr && tweener->Handle == Handle && !tweener->bKilled)
        {
            Handle.Invalidate();
            tweener->Kill(bCompleted);
            return true;
        }
    }

    Handle.Invalidate();
    return false;
}

bool FTweenManager::KillTweens(UObject* Target, bool bCompleted)
{
    if (Target == nullptr)
        return false;

    bool flag = false;
    int32 cnt = TotalActiveTweens;
    for (int32 i = 0; i < cnt; i++)
    {
        FGTweener* tweener = ActiveTweens[i];
        if (tweener != nullptr && tweener->Target.Get() == Target && !tweener->bKilled)
        {
            tweener->Kill(bCompleted);
            flag = true;
        }
    }

    return flag;
}

FGTweener* FTweenManager::GetTween(FTweenerHandle const& Handle)
{
    if (!Handle.IsValid())
        return nullptr;

    int32 cnt = TotalActiveTweens;
    for (int32 i = 0; i < cnt; i++)
    {
        FGTweener* tweener = ActiveTweens[i];
        if (tweener != nullptr && tweener->Handle == Handle && !tweener->bKilled)
        {
            return tweener;
        }
    }

    return nullptr;
}

FGTweener* FTweenManager::GetTween(UObject* Target)
{
    if (Target == nullptr)
        return nullptr;

    int32 cnt = TotalActiveTweens;
    for (int32 i = 0; i < cnt; i++)
    {
        FGTweener* tweener = ActiveTweens[i];
        if (tweener != nullptr && tweener->Target.Get() == Target && !tweener->bKilled)
        {
            return tweener;
        }
    }

    return nullptr;
}

void FTweenManager::Tick(float DeltaTime)
{
    int32 cnt = TotalActiveTweens;
    int32 freePosStart = -1;
    for (int32 i = 0; i < cnt; i++)
    {
        FGTweener* tweener = ActiveTweens[i];
        if (tweener == nullptr)
        {
            if (freePosStart == -1)
                freePosStart = i;
        }
        else if (tweener->bKilled)
        {
            tweener->Reset();
            TweenerPool.Add(tweener);
            ActiveTweens[i] = nullptr;

            if (freePosStart == -1)
                freePosStart = i;
        }
        else
        {
            if (tweener->Target.IsStale())
                tweener->bKilled = true;
            else if (!tweener->bPaused)
                tweener->Update(DeltaTime);

            if (freePosStart != -1)
            {
                ActiveTweens[freePosStart] = tweener;
                ActiveTweens[i] = nullptr;
                freePosStart++;
            }
        }
    }

    if (freePosStart >= 0)
    {
        if (TotalActiveTweens != cnt) //new tweens added
        {
            int32 j = cnt;
            cnt = TotalActiveTweens - cnt;
            for (int32 i = 0; i < cnt; i++)
                ActiveTweens[freePosStart++] = ActiveTweens[j++];
        }
        TotalActiveTweens = freePosStart;
    }
}