#include "UI/Gears/GearDisplay2.h"
#include "UI/GController.h"
#include "Utils/ByteBuffer.h"

FGearDisplay2::FGearDisplay2(UGObject* InOwner) :
    FGearBase(InOwner),
    Condition(0),
    Visible(0)
{
    Type = EType::Display2;
}

FGearDisplay2::~FGearDisplay2()
{
}

void FGearDisplay2::Apply()
{
    if (Controller == nullptr || Pages.Num() == 0)
        Visible = 1;
    else
    {
        if (Pages.Contains(Controller->GetSelectedPageID()))
            Visible = 1;
        else
            Visible = 0;
    }
}

bool FGearDisplay2::Evaluate(bool bConnected)
{
    bool v = Controller == nullptr || Visible > 0;
    if (Condition == 0)
        v = v && bConnected;
    else
        v = v || bConnected;
    return v;
}

void FGearDisplay2::UpdateState()
{
}

void FGearDisplay2::AddStatus(const FString& PageID, FByteBuffer* Buffer)
{
}

void FGearDisplay2::Init()
{
    Pages.Reset();
}
