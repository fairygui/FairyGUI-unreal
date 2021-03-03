#include "UI/Gears/GearAnimation.h"
#include "UI/GObject.h"
#include "UI/GController.h"
#include "Utils/ByteBuffer.h"

FGearAnimation::FValue::FValue() :
    bPlaying(false), Frame(0)
{
}

FGearAnimation::FGearAnimation(UGObject* InOwner) : FGearBase(InOwner)
{
    Type = EType::Animation;
}

FGearAnimation::~FGearAnimation()
{
}

void FGearAnimation::Init()
{
    Default.bPlaying = Owner->GetProp<bool>(EObjectPropID::Playing);
    Default.Frame = Owner->GetProp<int32>(EObjectPropID::Frame);
    Storage.Reset();
}

void FGearAnimation::AddStatus(const FString& PageID, FByteBuffer* Buffer)
{
    FValue Value;
    Value.bPlaying = Buffer->ReadBool();
    Value.Frame = Buffer->ReadInt();
    if (PageID.IsEmpty())
        Default = Value;
    else
        Storage.Add(PageID, MoveTemp(Value));
}

void FGearAnimation::Apply()
{
    Owner->bGearLocked = true;

    FValue* Value = Storage.Find(Controller->GetSelectedPageID());
    if (Value == nullptr)
        Value = &Default;

    Owner->SetProp(EObjectPropID::Playing, FNVariant(Value->bPlaying));
    Owner->SetProp(EObjectPropID::Frame, FNVariant(Value->Frame));

    Owner->bGearLocked = false;
}

void FGearAnimation::UpdateState()
{
    FValue Value;
    Value.bPlaying = Owner->GetProp<bool>(EObjectPropID::Playing);
    Value.Frame = Owner->GetProp<int32>(EObjectPropID::Frame);
    Storage.Add(Controller->GetSelectedPageID(), MoveTemp(Value));
}