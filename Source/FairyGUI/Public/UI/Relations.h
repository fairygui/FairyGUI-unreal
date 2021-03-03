#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RelationItem.h"

class UGObject;
class FByteBuffer;

class FAIRYGUI_API FRelations
{
public:
    FRelations(UGObject* InOwner);
    ~FRelations();

    void Add(UGObject* InTarget, ERelationType RelationType);
    void Add(UGObject* InTarget, ERelationType RelationType, bool bUsePercent);
    void Remove(UGObject* InTarget, ERelationType RelationType);
    bool Contains(UGObject* InTarget);
    void ClearFor(UGObject* InTarget);
    void ClearAll();
    void CopyFrom(const FRelations& Source);
    void OnOwnerSizeChanged(const FVector2D& Delta, bool bApplyPivot);
    bool IsEmpty() const;
    void Setup(FByteBuffer* Buffer, bool bParentToChild);

    UGObject* Handling;

private:
    UGObject* Owner;
    TIndirectArray<FRelationItem> Items;
};