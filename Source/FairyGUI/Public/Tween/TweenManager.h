#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Tickable.h"
#include "GTweener.h"

class UGObject;

class FAIRYGUI_API FTweenManager : public FTickableGameObject
{
public:
    static FTweenManager Singleton;

    FTweenManager();
    virtual ~FTweenManager();
    void Reset();

    FGTweener* CreateTween();

    bool IsTweening(FTweenerHandle const& Handle) { return GetTween(Handle) != nullptr; }
    bool IsTweening(UObject* Target) { return GetTween(Target) != nullptr; }

    bool KillTween(FTweenerHandle & Handle, bool bCompleted);
    bool KillTweens(UObject* Target, bool bCompleted);

    FGTweener* GetTween(FTweenerHandle const& Handle);
    FGTweener* GetTween(UObject* Target);

    void Tick(float DeltaTime);
    TStatId GetStatId() const {
        return TStatId();
    }

private:
    FGTweener** ActiveTweens;
    TArray<FGTweener*> TweenerPool;
    int32 TotalActiveTweens;
    int32 ArrayLength;
    uint32 TweenerInstanceCount;
};