#pragma once

#include "GearBase.h"

class FAIRYGUI_API FGearDisplay2 : public FGearBase
{
public:
    FGearDisplay2(UGObject* InOwner);
    virtual ~FGearDisplay2();

    virtual void Apply() override;
    virtual void UpdateState() override;
    bool Evaluate(bool bConnected);

    TArray<FString> Pages;
    int32 Condition;

protected:
    virtual void AddStatus(const FString& pageID, FByteBuffer* Buffer) override;
    virtual void Init() override;

private:
    int32 Visible;
};