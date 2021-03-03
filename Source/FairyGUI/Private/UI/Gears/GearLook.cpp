#include "UI/Gears/GearLook.h"
#include "UI/GObject.h"
#include "UI/UIPackage.h"
#include "UI/GController.h"
#include "Tween/GTween.h"
#include "Utils/ByteBuffer.h"

FGearLook::FValue::FValue() :
    Alpha(0),
    Rotation(0),
    bGrayed(false),
    bTouchable(false)
{
}

FGearLook::FGearLook(UGObject* InOwner) : FGearBase(InOwner)
{
    Type = EType::Look;
}

FGearLook::~FGearLook()
{
}

void FGearLook::Init()
{
    Default.Alpha = Owner->GetAlpha();
    Default.Rotation = Owner->GetRotation();
    Default.bGrayed = Owner->IsGrayed();
    Default.bTouchable = Owner->IsTouchable();
    Storage.Reset();
}

void FGearLook::AddStatus(const FString& PageID, FByteBuffer* Buffer)
{
    FValue Value;
    Value.Alpha = Buffer->ReadFloat();
    Value.Rotation = Buffer->ReadFloat();
    Value.bGrayed = Buffer->ReadBool();
    Value.bTouchable = Buffer->ReadBool();

    if (PageID.IsEmpty())
        Default = Value;
    else
        Storage.Add(PageID, MoveTemp(Value));
}

void FGearLook::Apply()
{
    FValue* Value = Storage.Find(Controller->GetSelectedPageID());
    if (Value == nullptr)
        Value = &Default;

    if (TweenConfig.IsSet() && TweenConfig->bTween && UUIPackage::Constructing == 0 && !bDisableAllTweenEffect)
    {
        Owner->bGearLocked = true;
        Owner->SetGrayed(Value->bGrayed);
        Owner->SetTouchable(Value->bTouchable);
        Owner->bGearLocked = false;

        FGTweener* tweener = FGTween::GetTween(TweenConfig->Handle);
        if (tweener != nullptr)
        {
            if (tweener->EndValue.X != Value->Alpha || tweener->EndValue.Y != Value->Rotation)
                tweener->Kill(true);
            else
                return;
        }

        bool a = Value->Alpha != Owner->GetAlpha();
        bool b = Value->Rotation != Owner->GetRotation();
        if (a || b)
        {
            if (Owner->CheckGearController(0, Controller))
                TweenConfig->DisplayLockToken = Owner->AddDisplayLock();

            TweenConfig->Handle = FGTween::To(FVector2D(Owner->GetAlpha(), Owner->GetRotation()), FVector2D(Value->Alpha, Value->Rotation), TweenConfig->Duration)
                ->SetDelay(TweenConfig->Delay)
                ->SetEase(TweenConfig->EaseType)
                ->SetTarget(Owner)
                ->SetUserData(FNVariant((a ? 1 : 0) + (b ? 2 : 0)))
                ->OnUpdate(FTweenDelegate::CreateRaw(this, &FGearLook::OnTweenUpdate))
                ->OnComplete(FSimpleDelegate::CreateRaw(this, &FGearLook::OnTweenComplete))
                ->GetHandle();
        }
    }
    else
    {
        Owner->bGearLocked = true;
        Owner->SetAlpha(Value->Alpha);
        Owner->SetRotation(Value->Rotation);
        Owner->SetGrayed(Value->bGrayed);
        Owner->SetTouchable(Value->bTouchable);
        Owner->bGearLocked = false;
    }
}

void FGearLook::OnTweenUpdate(FGTweener* Tweener)
{
    int32 flag = Tweener->GetUserData().AsInt();
    Owner->bGearLocked = true;

    if ((flag & 1) != 0)
        Owner->SetAlpha(Tweener->Value.X);
    if ((flag & 2) != 0)
        Owner->SetRotation(Tweener->Value.Y);
    Owner->bGearLocked = false;
}

void FGearLook::OnTweenComplete()
{
    if (TweenConfig->DisplayLockToken != 0)
    {
        Owner->ReleaseDisplayLock(TweenConfig->DisplayLockToken);
        TweenConfig->DisplayLockToken = 0;
    }
    TweenConfig->Handle.Invalidate();
    Owner->DispatchEvent(FUIEvents::GearStop);
}

void FGearLook::UpdateState()
{
    FValue Value;
    Value.Alpha = Owner->GetAlpha();
    Value.Rotation = Owner->GetRotation();
    Value.bGrayed = Owner->IsGrayed();
    Value.bTouchable = Owner->IsTouchable();
    Storage.Add(Controller->GetSelectedPageID(), MoveTemp(Value));
}
