#pragma once

#include "GearBase.h"

class FAIRYGUI_API FGearIcon : public FGearBase
{
public:
    FGearIcon(UGObject* InOwner);
    virtual ~FGearIcon();

    virtual void Apply() override;
    virtual void UpdateState() override;

protected:
    virtual void AddStatus(const FString& PageID, FByteBuffer* Buffer) override;
    virtual void Init() override;

private:
    TMap<FString, FString> Storage;
    FString Default;
};