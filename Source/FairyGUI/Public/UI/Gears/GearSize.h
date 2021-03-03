#pragma once

#include "GearBase.h"

class FAIRYGUI_API FGearSize : public FGearBase
{
public:
    FGearSize(UGObject* InOwner);
    virtual ~FGearSize();

    virtual void Apply() override;
    virtual void UpdateState() override;
    virtual void UpdateFromRelations(const FVector2D& Delta) override;

protected:
    virtual void AddStatus(const FString& PageID, FByteBuffer* Buffer) override;
    virtual void Init() override;

private:
    void OnTweenUpdate(FGTweener* Tweener);
    void OnTweenComplete();

    TMap<FString, FVector4> Storage;
    FVector4 Default;
};