#include "UI/Gears/GearIcon.h"
#include "UI/GObject.h"
#include "UI/GController.h"
#include "Utils/ByteBuffer.h"

FGearIcon::FGearIcon(UGObject * InOwner) :FGearBase(InOwner)
{
    Type = EType::Icon;
}

FGearIcon::~FGearIcon()
{
}

void FGearIcon::Init()
{
    Default = Owner->GetIcon();
    Storage.Reset();
}

void FGearIcon::AddStatus(const FString& PageID, FByteBuffer* Buffer)
{
    if (PageID.IsEmpty())
        Default = Buffer->ReadS();
    else
        Storage.Add(PageID, Buffer->ReadS());
}

void FGearIcon::Apply()
{
    FString* Value = Storage.Find(Controller->GetSelectedPageID());
    if (Value == nullptr)
        Value = &Default;

    Owner->bGearLocked = true;
    Owner->SetIcon(*Value);
    Owner->bGearLocked = false;
}

void FGearIcon::UpdateState()
{
    Storage.Add(Controller->GetSelectedPageID(), Owner->GetIcon());
}
