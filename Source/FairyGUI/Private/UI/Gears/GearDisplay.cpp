#include "UI/Gears/GearDisplay.h"
#include "UI/GController.h"
#include "Utils/ByteBuffer.h"

FGearDisplay::FGearDisplay(UGObject* InOwner) :
    FGearBase(InOwner),
    Visible(0),
    DisplayLockToken(1)
{
    Type = EType::Display;
}

FGearDisplay::~FGearDisplay()
{
}

void FGearDisplay::Apply()
{
    DisplayLockToken++;
    if (DisplayLockToken == 0)
        DisplayLockToken = 1;

    if (Pages.Num() == 0)
        Visible = 1;
    else
    {
        if (Pages.Contains(Controller->GetSelectedPageID()))
            Visible = 1;
        else
            Visible = 0;
    }
}

void FGearDisplay::UpdateState()
{
}

void FGearDisplay::AddStatus(const FString& PageID, FByteBuffer* Buffer)
{
}

void FGearDisplay::Init()
{
    Pages.Reset();
}

uint32 FGearDisplay::AddLock()
{
    Visible++;
    return DisplayLockToken;
}

void FGearDisplay::ReleaseLock(uint32 token)
{
    if (token == DisplayLockToken)
        Visible--;
}

bool FGearDisplay::IsConnected()
{
    return Controller == nullptr || Visible > 0;
}