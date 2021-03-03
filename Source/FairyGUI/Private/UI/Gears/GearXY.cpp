#include "UI/Gears/GearXY.h"
#include "UI/GComponent.h"
#include "UI/UIPackage.h"
#include "UI/GController.h"
#include "Tween/GTween.h"
#include "Utils/ByteBuffer.h"

FGearXY::FGearXY(UGObject* InOwner) :
    FGearBase(InOwner),
    bPositionsInPercent(false)
{
    Type = EType::XY;
}

FGearXY::~FGearXY()
{
}

void FGearXY::Init()
{
    Default = FVector4(Owner->GetX(),
        Owner->GetY(),
        Owner->GetX() / Owner->GetParent()->GetWidth(),
        Owner->GetY() / Owner->GetParent()->GetHeight());
    Storage.Reset();
}

void FGearXY::AddStatus(const FString& PageID, FByteBuffer* Buffer)
{
    FVector4 Value;
    Value.X = Buffer->ReadInt();
    Value.Y = Buffer->ReadInt();

    if (PageID.IsEmpty())
        Default = Value;
    else
        Storage.Add(PageID, MoveTemp(Value));
}

void FGearXY::AddExtStatus(const FString& PageID, FByteBuffer* Buffer)
{
    FVector4* Value = PageID.IsEmpty() ? &Default : Storage.Find(Controller->GetSelectedPageID());
    Value->Z = Buffer->ReadFloat();
    Value->W = Buffer->ReadFloat();
}

void FGearXY::Apply()
{
    FVector4* Value = Storage.Find(Controller->GetSelectedPageID());
    if (Value == nullptr)
        Value = &Default;

    FVector2D EndPt;

    if (bPositionsInPercent && Owner->GetParent())
    {
        EndPt.X = Value->Z * Owner->GetParent()->GetWidth();
        EndPt.Y = Value->W * Owner->GetParent()->GetHeight();
    }
    else
    {
        EndPt.X = Value->X;
        EndPt.Y = Value->Y;
    }

    if (TweenConfig.IsSet() && TweenConfig->bTween && UUIPackage::Constructing == 0 && !bDisableAllTweenEffect)
    {
        FGTweener* tweener = FGTween::GetTween(TweenConfig->Handle);
        if (tweener != nullptr)
        {
            if (tweener->EndValue.GetVec2() != EndPt)
                tweener->Kill(true);
            else
                return;
        }

        FVector2D OriginPt(Owner->GetX(), Owner->GetY());

        if (OriginPt != EndPt)
        {
            if (Owner->CheckGearController(0, Controller))
                TweenConfig->DisplayLockToken = Owner->AddDisplayLock();

            TweenConfig->Handle = FGTween::To(OriginPt, EndPt, TweenConfig->Duration)
                ->SetDelay(TweenConfig->Delay)
                ->SetEase(TweenConfig->EaseType)
                ->SetTarget(Owner)
                ->OnUpdate(FTweenDelegate::CreateRaw(this, &FGearXY::OnTweenUpdate))
                ->OnComplete(FSimpleDelegate::CreateRaw(this, &FGearXY::OnTweenComplete))
                ->GetHandle();
        }
    }
    else
    {
        Owner->bGearLocked = true;
        Owner->SetPosition(EndPt);
        Owner->bGearLocked = false;
    }
}

void FGearXY::OnTweenUpdate(FGTweener* Tweener)
{
    Owner->bGearLocked = true;
    Owner->SetPosition(Tweener->Value.GetVec2());
    Owner->bGearLocked = false;
}

void FGearXY::OnTweenComplete()
{
    if (TweenConfig->DisplayLockToken != 0)
    {
        Owner->ReleaseDisplayLock(TweenConfig->DisplayLockToken);
        TweenConfig->DisplayLockToken = 0;
    }
    TweenConfig->Handle.Invalidate();
    Owner->DispatchEvent(FUIEvents::GearStop);
}

void FGearXY::UpdateState()
{
    Storage.Add(Controller->GetSelectedPageID(), FVector4(
        Owner->GetX(),
        Owner->GetY(),
        Owner->GetX() / Owner->GetParent()->GetWidth(),
        Owner->GetY() / Owner->GetParent()->GetHeight()));
}

void FGearXY::UpdateFromRelations(const FVector2D& Delta)
{
    if (Controller != nullptr && Storage.Num() > 0 && !bPositionsInPercent)
    {
        for (auto It = Storage.CreateIterator(); It; ++It)
        {
            It->Value = FVector4(It->Value.X + Delta.X, It->Value.Y + Delta.Y,
                It->Value.Z, It->Value.W);
        }
        Default.X += Delta.X;
        Default.Y += Delta.Y;

        UpdateState();
    }
}
