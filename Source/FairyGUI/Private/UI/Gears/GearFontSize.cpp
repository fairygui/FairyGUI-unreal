#include "UI/Gears/GearFontSize.h"
#include "UI/GObject.h"
#include "UI/GController.h"
#include "Utils/ByteBuffer.h"

FGearFontSize::FGearFontSize(UGObject* InOwner) : FGearBase(InOwner)
{
    Type = EType::FontSize;
}

FGearFontSize::~FGearFontSize()
{
}

void FGearFontSize::Init()
{
    Default = Owner->GetProp<int32>(EObjectPropID::FontSize);
    Storage.Reset();
}

void FGearFontSize::AddStatus(const FString& PageID, FByteBuffer* Buffer)
{
    if (PageID.IsEmpty())
        Default = Buffer->ReadInt();
    else
        Storage.Add(PageID, Buffer->ReadInt());
}

void FGearFontSize::Apply()
{
    int32* Value = Storage.Find(Controller->GetSelectedPageID());
    if (Value == nullptr)
        Value = &Default;

    Owner->bGearLocked = true;
    Owner->SetProp(EObjectPropID::FontSize, FNVariant(*Value));
    Owner->bGearLocked = false;
}

void FGearFontSize::UpdateState()
{
    Storage.Add(Controller->GetSelectedPageID(), Owner->GetProp<int32>(EObjectPropID::FontSize));
}
