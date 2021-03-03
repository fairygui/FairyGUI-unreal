#pragma once

#include "GearBase.h"

class FAIRYGUI_API FGearLook : public FGearBase
{
public:
    FGearLook(UGObject* InOwner);
    virtual ~FGearLook();

    virtual void Apply() override;
    virtual void UpdateState() override;

protected:
    virtual void AddStatus(const FString& PageID, FByteBuffer* Buffer) override;
    virtual void Init() override;

private:
    void OnTweenUpdate(FGTweener* Tweener);
    void OnTweenComplete();

    struct FValue
    {
        float Alpha;
        float Rotation;
        bool bGrayed;
        bool bTouchable;

        FValue();
    };

    TMap<FString, FValue> Storage;
    FValue Default;
};