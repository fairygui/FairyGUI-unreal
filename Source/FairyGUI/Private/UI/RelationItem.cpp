#include "UI/RelationItem.h"
#include "UI/Relations.h"
#include "UI/GComponent.h"
#include "UI/GGroup.h"
#include "UI/Transition.h"

FRelationItem::FRelationItem(UGObject* InOwner) :
    TargetData(ForceInit)
{
    Owner = InOwner;
}

FRelationItem::~FRelationItem()
{
    ReleaseRefTarget();
}

void FRelationItem::SetTarget(UGObject* InTarget)
{
    if (Target.Get() != InTarget)
    {
        ReleaseRefTarget();
        Target = InTarget;
        if (InTarget)
            AddRefTarget(InTarget);
    }
}

void FRelationItem::Add(ERelationType RelationType, bool bUsePercent)
{
    if (RelationType == ERelationType::Size)
    {
        Add(ERelationType::Width, bUsePercent);
        Add(ERelationType::Height, bUsePercent);
        return;
    }

    for (auto& it : Defs)
    {
        if (it.Type == RelationType)
            return;
    }

    InternalAdd(RelationType, bUsePercent);
}

void FRelationItem::InternalAdd(ERelationType RelationType, bool bUsePercent)
{
    if (RelationType == ERelationType::Size)
    {
        InternalAdd(ERelationType::Width, bUsePercent);
        InternalAdd(ERelationType::Height, bUsePercent);
        return;
    }

    FRelationDef info;
    info.bPercent = bUsePercent;
    info.Type = RelationType;
    info.Axis = (RelationType <= ERelationType::Right_Right || RelationType == ERelationType::Width || (RelationType >= ERelationType::LeftExt_Left && RelationType <= ERelationType::RightExt_Right)) ? 0 : 1;
    Defs.Add(info);
}

void FRelationItem::Remove(ERelationType RelationType)
{
    if (RelationType == ERelationType::Size)
    {
        Remove(ERelationType::Width);
        Remove(ERelationType::Height);
        return;
    }

    int32 i = 0;
    for (auto& def : Defs)
    {
        if (def.Type == RelationType)
        {
            Defs.RemoveAt(i);
            break;
        }
        i++;
    }
}

void FRelationItem::CopyFrom(const FRelationItem& Source)
{
    SetTarget(Source.Target.Get());

    Defs.Reset();
    for (auto& it : Source.Defs)
        Defs.Add(it);
}

bool FRelationItem::IsEmpty() const
{
    return Defs.Num() == 0;
}

void FRelationItem::ApplyOnSelfSizeChanged(float DeltaWidth, float DeltaHeight, bool bApplyPivot)
{
    if (!Target.IsValid() || Defs.Num() == 0)
        return;

    FVector2D Pos = Owner->Position;

    for (auto& it : Defs)
    {
        switch (it.Type)
        {
        case ERelationType::Center_Center:
            Owner->SetX(Owner->Position.X - (0.5 - (bApplyPivot ? Owner->Pivot.X : 0)) * DeltaWidth);
            break;

        case ERelationType::Right_Center:
        case ERelationType::Right_Left:
        case ERelationType::Right_Right:
            Owner->SetX(Owner->Position.X - (1 - (bApplyPivot ? Owner->Pivot.X : 0)) * DeltaWidth);
            break;

        case ERelationType::Middle_Middle:
            Owner->SetY(Owner->Position.Y - (0.5 - (bApplyPivot ? Owner->Pivot.Y : 0)) * DeltaHeight);
            break;

        case ERelationType::Bottom_Middle:
        case ERelationType::Bottom_Top:
        case ERelationType::Bottom_Bottom:
            Owner->SetY(Owner->Position.Y - (1 - (bApplyPivot ? Owner->Pivot.Y : 0)) * DeltaHeight);
            break;

        default:
            break;
        }
    }

    if (Pos != Owner->Position)
    {
        FVector2D Delta = Owner->Position - Pos;

        Owner->UpdateGearFromRelations(1, Delta);

        if (Owner->Parent.IsValid())
        {
            const TArray<UTransition*>& arr = Owner->Parent->GetTransitions();
            for (auto& it : arr)
                it->UpdateFromRelations(Owner->ID, Delta);
        }
    }
}

void FRelationItem::ApplyOnXYChanged(UGObject* InTarget, const FRelationDef& info, float dx, float dy)
{
    float tmp;

    switch (info.Type)
    {
    case ERelationType::Left_Left:
    case ERelationType::Left_Center:
    case ERelationType::Left_Right:
    case ERelationType::Center_Center:
    case ERelationType::Right_Left:
    case ERelationType::Right_Center:
    case ERelationType::Right_Right:
        Owner->SetX(Owner->Position.X + dx);
        break;

    case ERelationType::Top_Top:
    case ERelationType::Top_Middle:
    case ERelationType::Top_Bottom:
    case ERelationType::Middle_Middle:
    case ERelationType::Bottom_Top:
    case ERelationType::Bottom_Middle:
    case ERelationType::Bottom_Bottom:
        Owner->SetY(Owner->Position.Y + dy);
        break;

    case ERelationType::Width:
    case ERelationType::Height:
        break;

    case ERelationType::LeftExt_Left:
    case ERelationType::LeftExt_Right:
        if (Owner != InTarget->Parent)
        {
            tmp = Owner->GetXMin();
            Owner->SetWidth(Owner->RawSize.X - dx);
            Owner->SetXMin(tmp + dx);
        }
        else
            Owner->SetWidth(Owner->RawSize.X - dx);
        break;

    case ERelationType::RightExt_Left:
    case ERelationType::RightExt_Right:
        if (Owner != InTarget->Parent)
        {
            tmp = Owner->GetXMin();
            Owner->SetWidth(Owner->RawSize.X + dx);
            Owner->SetXMin(tmp);
        }
        else
            Owner->SetWidth(Owner->RawSize.X + dx);
        break;

    case ERelationType::TopExt_Top:
    case ERelationType::TopExt_Bottom:
        if (Owner != InTarget->Parent)
        {
            tmp = Owner->GetYMin();
            Owner->SetHeight(Owner->RawSize.Y - dy);
            Owner->SetYMin(tmp + dy);
        }
        else
            Owner->SetHeight(Owner->RawSize.Y - dy);
        break;

    case ERelationType::BottomExt_Top:
    case ERelationType::BottomExt_Bottom:
        if (Owner != InTarget->Parent)
        {
            tmp = Owner->GetYMin();
            Owner->SetHeight(Owner->RawSize.Y + dy);
            Owner->SetYMin(tmp);
        }
        else
            Owner->SetHeight(Owner->RawSize.Y + dy);
        break;

    default:
        break;
    }
}

void FRelationItem::ApplyOnSizeChanged(UGObject* InTarget, const FRelationDef& info)
{
    float pos = 0, pivot = 0, delta = 0;
    if (info.Axis == 0)
    {
        if (InTarget != Owner->Parent)
        {
            pos = InTarget->Position.X;
            if (InTarget->bPivotAsAnchor)
                pivot = InTarget->Pivot.X;
        }

        if (info.bPercent)
        {
            if (TargetData.Z != 0)
                delta = InTarget->Size.X / TargetData.Z;
        }
        else
            delta = InTarget->Size.X - TargetData.Z;
    }
    else
    {
        if (InTarget != Owner->Parent)
        {
            pos = InTarget->Position.Y;
            if (InTarget->bPivotAsAnchor)
                pivot = InTarget->Pivot.Y;
        }

        if (info.bPercent)
        {
            if (TargetData.W != 0)
                delta = InTarget->Size.Y / TargetData.W;
        }
        else
            delta = InTarget->Size.Y - TargetData.W;
    }

    float v, tmp;
    switch (info.Type)
    {
    case ERelationType::Left_Left:
        if (info.bPercent)
            Owner->SetXMin(pos + (Owner->GetXMin() - pos) * delta);
        else if (pivot != 0)
            Owner->SetX(Owner->Position.X + delta * (-pivot));
        break;
    case ERelationType::Left_Center:
        if (info.bPercent)
            Owner->SetXMin(pos + (Owner->GetXMin() - pos) * delta);
        else
            Owner->SetX(Owner->Position.X + delta * (0.5f - pivot));
        break;
    case ERelationType::Left_Right:
        if (info.bPercent)
            Owner->SetXMin(pos + (Owner->GetXMin() - pos) * delta);
        else
            Owner->SetX(Owner->Position.X + delta * (1 - pivot));
        break;
    case ERelationType::Center_Center:
        if (info.bPercent)
            Owner->SetXMin(pos + (Owner->GetXMin() + Owner->RawSize.X * 0.5f - pos) * delta - Owner->RawSize.X * 0.5f);
        else
            Owner->SetX(Owner->Position.X + delta * (0.5f - pivot));
        break;
    case ERelationType::Right_Left:
        if (info.bPercent)
            Owner->SetXMin(pos + (Owner->GetXMin() + Owner->RawSize.X - pos) * delta - Owner->RawSize.X);
        else if (pivot != 0)
            Owner->SetX(Owner->Position.X + delta * (-pivot));
        break;
    case ERelationType::Right_Center:
        if (info.bPercent)
            Owner->SetXMin(pos + (Owner->GetXMin() + Owner->RawSize.X - pos) * delta - Owner->RawSize.X);
        else
            Owner->SetX(Owner->Position.X + delta * (0.5f - pivot));
        break;
    case ERelationType::Right_Right:
        if (info.bPercent)
            Owner->SetXMin(pos + (Owner->GetXMin() + Owner->RawSize.X - pos) * delta - Owner->RawSize.X);
        else
            Owner->SetX(Owner->Position.X + delta * (1 - pivot));
        break;

    case ERelationType::Top_Top:
        if (info.bPercent)
            Owner->SetYMin(pos + (Owner->GetYMin() - pos) * delta);
        else if (pivot != 0)
            Owner->SetY(Owner->Position.Y + delta * (-pivot));
        break;
    case ERelationType::Top_Middle:
        if (info.bPercent)
            Owner->SetYMin(pos + (Owner->GetYMin() - pos) * delta);
        else
            Owner->SetY(Owner->Position.Y + delta * (0.5f - pivot));
        break;
    case ERelationType::Top_Bottom:
        if (info.bPercent)
            Owner->SetYMin(pos + (Owner->GetYMin() - pos) * delta);
        else
            Owner->SetY(Owner->Position.Y + delta * (1 - pivot));
        break;
    case ERelationType::Middle_Middle:
        if (info.bPercent)
            Owner->SetYMin(pos + (Owner->GetYMin() + Owner->RawSize.Y * 0.5f - pos) * delta - Owner->RawSize.Y * 0.5f);
        else
            Owner->SetY(Owner->Position.Y + delta * (0.5f - pivot));
        break;
    case ERelationType::Bottom_Top:
        if (info.bPercent)
            Owner->SetYMin(pos + (Owner->GetYMin() + Owner->RawSize.Y - pos) * delta - Owner->RawSize.Y);
        else if (pivot != 0)
            Owner->SetY(Owner->Position.Y + delta * (-pivot));
        break;
    case ERelationType::Bottom_Middle:
        if (info.bPercent)
            Owner->SetYMin(pos + (Owner->GetYMin() + Owner->RawSize.Y - pos) * delta - Owner->RawSize.Y);
        else
            Owner->SetY(Owner->Position.Y + delta * (0.5f - pivot));
        break;
    case ERelationType::Bottom_Bottom:
        if (info.bPercent)
            Owner->SetYMin(pos + (Owner->GetYMin() + Owner->RawSize.Y - pos) * delta - Owner->RawSize.Y);
        else
            Owner->SetY(Owner->Position.Y + delta * (1 - pivot));
        break;

    case ERelationType::Width:
        if (Owner->bUnderConstruct && Owner == InTarget->Parent)
            v = Owner->SourceSize.X - InTarget->InitSize.X;
        else
            v = Owner->RawSize.X - TargetData.Z;
        if (info.bPercent)
            v = v * delta;
        if (InTarget == Owner->Parent)
        {
            if (Owner->bPivotAsAnchor)
            {
                tmp = Owner->GetXMin();
                Owner->SetSize(FVector2D(InTarget->Size.X + v, Owner->RawSize.Y), true);
                Owner->SetXMin(tmp);
            }
            else
                Owner->SetSize(FVector2D(InTarget->Size.X + v, Owner->RawSize.Y), true);
        }
        else
            Owner->SetWidth(InTarget->Size.X + v);
        break;
    case ERelationType::Height:
        if (Owner->bUnderConstruct && Owner == InTarget->Parent)
            v = Owner->SourceSize.Y - InTarget->InitSize.Y;
        else
            v = Owner->RawSize.Y - TargetData.W;
        if (info.bPercent)
            v = v * delta;
        if (InTarget == Owner->Parent)
        {
            if (Owner->bPivotAsAnchor)
            {
                tmp = Owner->GetYMin();
                Owner->SetSize(FVector2D(Owner->RawSize.X, InTarget->Size.Y + v), true);
                Owner->SetYMin(tmp);
            }
            else
                Owner->SetSize(FVector2D(Owner->RawSize.X, InTarget->Size.Y + v), true);
        }
        else
            Owner->SetHeight(InTarget->Size.Y + v);
        break;

    case ERelationType::LeftExt_Left:
        tmp = Owner->GetXMin();
        if (info.bPercent)
            v = pos + (tmp - pos) * delta - tmp;
        else
            v = delta * (-pivot);
        Owner->SetWidth(Owner->RawSize.X - v);
        Owner->SetXMin(tmp + v);
        break;
    case ERelationType::LeftExt_Right:
        tmp = Owner->GetXMin();
        if (info.bPercent)
            v = pos + (tmp - pos) * delta - tmp;
        else
            v = delta * (1 - pivot);
        Owner->SetWidth(Owner->RawSize.X - v);
        Owner->SetXMin(tmp + v);
        break;
    case ERelationType::RightExt_Left:
        tmp = Owner->GetXMin();
        if (info.bPercent)
            v = pos + (tmp + Owner->RawSize.X - pos) * delta - (tmp + Owner->RawSize.X);
        else
            v = delta * (-pivot);
        Owner->SetWidth(Owner->RawSize.X + v);
        Owner->SetXMin(tmp);
        break;
    case ERelationType::RightExt_Right:
        tmp = Owner->GetXMin();
        if (info.bPercent)
        {
            if (Owner == InTarget->Parent)
            {
                if (Owner->bUnderConstruct)
                    Owner->SetWidth(pos + InTarget->Size.X - InTarget->Size.X * pivot +
                    (Owner->SourceSize.X - pos - InTarget->InitSize.X + InTarget->InitSize.X * pivot) * delta);
                else
                    Owner->SetWidth(pos + (Owner->RawSize.X - pos) * delta);
            }
            else
            {
                v = pos + (tmp + Owner->RawSize.X - pos) * delta - (tmp + Owner->RawSize.X);
                Owner->SetWidth(Owner->RawSize.X + v);
                Owner->SetXMin(tmp);
            }
        }
        else
        {
            if (Owner == InTarget->Parent)
            {
                if (Owner->bUnderConstruct)
                    Owner->SetWidth(Owner->SourceSize.X + (InTarget->Size.X - InTarget->InitSize.X) * (1 - pivot));
                else
                    Owner->SetWidth(Owner->RawSize.X + delta * (1 - pivot));
            }
            else
            {
                v = delta * (1 - pivot);
                Owner->SetWidth(Owner->RawSize.X + v);
                Owner->SetXMin(tmp);
            }
        }
        break;
    case ERelationType::TopExt_Top:
        tmp = Owner->GetYMin();
        if (info.bPercent)
            v = pos + (tmp - pos) * delta - tmp;
        else
            v = delta * (-pivot);
        Owner->SetHeight(Owner->RawSize.Y - v);
        Owner->SetYMin(tmp + v);
        break;
    case ERelationType::TopExt_Bottom:
        tmp = Owner->GetYMin();
        if (info.bPercent)
            v = pos + (tmp - pos) * delta - tmp;
        else
            v = delta * (1 - pivot);
        Owner->SetHeight(Owner->RawSize.Y - v);
        Owner->SetYMin(tmp + v);
        break;
    case ERelationType::BottomExt_Top:
        tmp = Owner->GetYMin();
        if (info.bPercent)
            v = pos + (tmp + Owner->RawSize.Y - pos) * delta - (tmp + Owner->RawSize.Y);
        else
            v = delta * (-pivot);
        Owner->SetHeight(Owner->RawSize.Y + v);
        Owner->SetYMin(tmp);
        break;
    case ERelationType::BottomExt_Bottom:
        tmp = Owner->GetYMin();
        if (info.bPercent)
        {
            if (Owner == InTarget->Parent)
            {
                if (Owner->bUnderConstruct)
                    Owner->SetHeight(pos + InTarget->Size.Y - InTarget->Size.Y * pivot +
                    (Owner->SourceSize.Y - pos - InTarget->InitSize.Y + InTarget->InitSize.Y * pivot) * delta);
                else
                    Owner->SetHeight(pos + (Owner->RawSize.Y - pos) * delta);
            }
            else
            {
                v = pos + (tmp + Owner->RawSize.Y - pos) * delta - (tmp + Owner->RawSize.Y);
                Owner->SetHeight(Owner->RawSize.Y + v);
                Owner->SetYMin(tmp);
            }
        }
        else
        {
            if (Owner == InTarget->Parent)
            {
                if (Owner->bUnderConstruct)
                    Owner->SetHeight(Owner->SourceSize.Y + (InTarget->Size.Y - InTarget->InitSize.Y) * (1 - pivot));
                else
                    Owner->SetHeight(Owner->RawSize.Y + delta * (1 - pivot));
            }
            else
            {
                v = delta * (1 - pivot);
                Owner->SetHeight(Owner->RawSize.Y + v);
                Owner->SetYMin(tmp);
            }
        }
        break;
    default:
        break;
    }
}

void FRelationItem::AddRefTarget(UGObject* InTarget)
{
    if (!InTarget)
        return;

    if (InTarget != Owner->GetParent())
        PositionDelegateHandle = InTarget->OnPositionChanged().AddRaw(this, &FRelationItem::OnTargetXYChanged);
    SizeDelegateHandle = InTarget->OnSizeChanged().AddRaw(this, &FRelationItem::OnTargetSizeChanged);

    TargetData.X = InTarget->Position.X;
    TargetData.Y = InTarget->Position.Y;
    TargetData.Z = InTarget->Size.X;
    TargetData.W = InTarget->Size.Y;
}

void FRelationItem::ReleaseRefTarget()
{
    if (!Target.IsValid())
        return;

    Target->OnPositionChanged().Remove(PositionDelegateHandle);
    Target->OnSizeChanged().Remove(SizeDelegateHandle);
}

void FRelationItem::OnTargetXYChanged()
{
    if (Owner->Relations->Handling != nullptr || (Owner->Group.IsValid() && Owner->Group->Updating != 0))
    {
        TargetData.X = Target->Position.X;
        TargetData.Y = Target->Position.Y;
        return;
    }

    Owner->Relations->Handling = Target.Get();

    FVector2D Pos = Owner->Position;
    float dx = Target->Position.X - TargetData.X;
    float dy = Target->Position.Y - TargetData.Y;

    for (auto& it : Defs)
        ApplyOnXYChanged(Target.Get(), it, dx, dy);

    TargetData.X = Target->Position.X;
    TargetData.Y = Target->Position.Y;

    if (Pos != Owner->Position)
    {
        FVector2D Delta = Owner->Position - Pos;

        Owner->UpdateGearFromRelations(1, Delta);

        if (Owner->Parent.IsValid())
        {
            const TArray<UTransition*>& arr = Owner->Parent->GetTransitions();
            for (auto& it : arr)
                it->UpdateFromRelations(Owner->ID, Delta);
        }
    }

    Owner->Relations->Handling = nullptr;
}

void FRelationItem::OnTargetSizeChanged()
{
    if (Owner->Relations->Handling != nullptr
        || (Owner->Group.IsValid() && Owner->Group->Updating != 0))
    {
        TargetData.Z = Target->Size.X;
        TargetData.W = Target->Size.Y;
        return;
    }
    Owner->Relations->Handling = Target.Get();

    FVector2D Pos = Owner->Position;
    FVector2D RawSize = Owner->RawSize;

    for (auto& it : Defs)
        ApplyOnSizeChanged(Target.Get(), it);

    TargetData.Z = Target->Size.X;
    TargetData.W = Target->Size.Y;

    if (Pos != Owner->Position)
    {
        FVector2D Delta = Owner->Position - Pos;

        Owner->UpdateGearFromRelations(1, Delta);

        if (Owner->Parent.IsValid())
        {
            const TArray<UTransition*>& arr = Owner->Parent->GetTransitions();
            for (auto& it : arr)
                it->UpdateFromRelations(Owner->ID, Delta);
        }
    }

    if (RawSize != Owner->RawSize)
    {
        FVector2D Delta = Owner->RawSize - RawSize;

        Owner->UpdateGearFromRelations(2, Delta);
    }

    Owner->Relations->Handling = nullptr;
}