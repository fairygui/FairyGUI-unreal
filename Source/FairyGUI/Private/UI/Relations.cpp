#include "UI/Relations.h"
#include "UI/GComponent.h"
#include "Utils/ByteBuffer.h"

FRelations::FRelations(UGObject* InOwner) :
    Handling(nullptr)
{
    Owner = InOwner;
}

FRelations::~FRelations()
{
}

void FRelations::Add(UGObject * InTarget, ERelationType ERelationType)
{
    Add(InTarget, ERelationType, false);
}

void FRelations::Add(UGObject * InTarget, ERelationType ERelationType, bool bUsePercent)
{
    verifyf(InTarget, TEXT("target is null"));

    for (auto& it : Items)
    {
        if (it.GetTarget() == InTarget)
        {
            it.Add(ERelationType, bUsePercent);
            return;
        }
    }
    FRelationItem* newItem = new FRelationItem(Owner);
    newItem->SetTarget(InTarget);
    newItem->Add(ERelationType, bUsePercent);
    Items.Add(newItem);
}

void FRelations::Remove(UGObject * InTarget, ERelationType ERelationType)
{
    int32 Index = 0;
    while (Index < Items.Num())
    {
        FRelationItem& Item = Items[Index];
        if (Item.GetTarget() == InTarget)
        {
            Item.Remove(ERelationType);
            if (Item.IsEmpty())
            {
                Items.RemoveAt(Index);
            }
            else
                Index++;
        }
        else
            Index++;
    }
}

bool FRelations::Contains(UGObject * InTarget)
{
    for (auto& it : Items)
    {
        if (it.GetTarget() == InTarget)
            return true;
    }

    return false;
}

void FRelations::ClearFor(UGObject * InTarget)
{
    int32 Index = 0;
    while (Index < Items.Num())
    {
        FRelationItem& Item = Items[Index];
        if (Item.GetTarget() == InTarget)
            Items.RemoveAt(Index);
        else
            Index++;
    }
}

void FRelations::ClearAll()
{
    Items.Reset();
}

void FRelations::CopyFrom(const FRelations & Source)
{
    ClearAll();

    for (auto& it : Source.Items)
    {
        FRelationItem* item = new FRelationItem(Owner);
        item->CopyFrom(it);
        Items.Add(item);
    }
}

void FRelations::OnOwnerSizeChanged(const FVector2D& Delta, bool bApplyPivot)
{
    for (auto& it : Items)
        it.ApplyOnSelfSizeChanged(Delta.X, Delta.Y, bApplyPivot);
}

bool FRelations::IsEmpty() const
{
    return Items.Num() == 0;
}

void FRelations::Setup(FByteBuffer * Buffer, bool bParentToChild)
{
    int32 cnt = Buffer->ReadByte();
    UGObject* target;
    for (int32 i = 0; i < cnt; i++)
    {
        int16 targetIndex = Buffer->ReadShort();
        if (targetIndex == -1)
            target = Owner->GetParent();
        else if (bParentToChild)
            target = (Cast<UGComponent>(Owner))->GetChildAt(targetIndex);
        else
            target = Owner->GetParent()->GetChildAt(targetIndex);

        FRelationItem* newItem = new FRelationItem(Owner);
        newItem->SetTarget(target);
        Items.Add(newItem);

        int32 cnt2 = Buffer->ReadByte();
        for (int32 j = 0; j < cnt2; j++)
        {
            ERelationType rt = (ERelationType)Buffer->ReadByte();
            bool usePercent = Buffer->ReadBool();
            newItem->InternalAdd(rt, usePercent);
        }
    }
}