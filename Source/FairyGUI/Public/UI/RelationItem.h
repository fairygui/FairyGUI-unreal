#pragma once

#include "CoreMinimal.h"
#include "FieldTypes.h"

class UGObject;

struct FAIRYGUI_API FRelationDef
{
    bool bPercent;
    ERelationType Type;
    int32 Axis;

    FRelationDef() :
        bPercent(false), Type(ERelationType::Left_Left), Axis(0)
    {}
};

class FAIRYGUI_API FRelationItem
{
public:
    FRelationItem(UGObject* InOwner);
    ~FRelationItem();

    UGObject* GetTarget() const { return Target.Get(); }
    void SetTarget(UGObject* InTarget);

    void Add(ERelationType RelationType, bool bUsePercent);
    void InternalAdd(ERelationType RelationType, bool bUsePercent);
    void Remove(ERelationType RelationType);
    void CopyFrom(const FRelationItem& Source);
    bool IsEmpty() const;
    void ApplyOnSelfSizeChanged(float DeltaWidth, float DeltaHeight, bool bApplyPivot);

private:
    void ApplyOnXYChanged(UGObject* InTarget, const FRelationDef& info, float dx, float dy);
    void ApplyOnSizeChanged(UGObject* InTarget, const FRelationDef& info);
    void AddRefTarget(UGObject* InTarget);
    void ReleaseRefTarget();
    void OnTargetXYChanged();
    void OnTargetSizeChanged();

    UGObject* Owner;
    TWeakObjectPtr<UGObject> Target;
    TArray<FRelationDef> Defs;
    FVector4 TargetData;

    FDelegateHandle PositionDelegateHandle;
    FDelegateHandle SizeDelegateHandle;
};