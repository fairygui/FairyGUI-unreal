#include "UI/Gears/GearText.h"
#include "UI/GObject.h"
#include "UI/GController.h"
#include "Utils/ByteBuffer.h"

FGearText::FGearText(UGObject* InOwner) : FGearBase(InOwner)
{
    Type = EType::Text;
}

FGearText::~FGearText()
{
}

void FGearText::Init()
{
    Default = Owner->GetText();
    Storage.Reset();
}

void FGearText::AddStatus(const FString& PageID, FByteBuffer* Buffer)
{
    if (PageID.IsEmpty())
        Default = Buffer->ReadS();
    else
        Storage.Add(PageID, Buffer->ReadS());
}

void FGearText::Apply()
{
    FString* Value = Storage.Find(Controller->GetSelectedPageID());
    if (Value == nullptr)
        Value = &Default;

    Owner->bGearLocked = true;
    Owner->SetText(*Value);
    Owner->bGearLocked = false;
}

void FGearText::UpdateState()
{
    Storage.Add(Controller->GetSelectedPageID(), Owner->GetText());
}
