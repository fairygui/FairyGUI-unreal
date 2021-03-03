#pragma once

#include "GearBase.h"

class FAIRYGUI_API FGearDisplay : public FGearBase
{
public:
    FGearDisplay(UGObject* InOwner);
    virtual ~FGearDisplay();

    virtual void Apply() override;
    virtual void UpdateState() override;

    uint32 AddLock();
    void ReleaseLock(uint32 Token);
    bool IsConnected();

    TArray<FString> Pages;

protected:
    virtual void AddStatus(const FString& PageID, FByteBuffer* Buffer) override;
    virtual void Init() override;

private:
    int32 Visible;
    uint32 DisplayLockToken;
};