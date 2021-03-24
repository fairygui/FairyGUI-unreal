#include "UI/Gears/GearBase.h"
#include "UI/Gears/GearDisplay.h"
#include "UI/Gears/GearAnimation.h"
#include "UI/Gears/GearColor.h"
#include "UI/Gears/GearDisplay2.h"
#include "UI/Gears/GearFontSize.h"
#include "UI/Gears/GearIcon.h"
#include "UI/Gears/GearLook.h"
#include "UI/Gears/GearSize.h"
#include "UI/Gears/GearText.h"
#include "UI/Gears/GearXY.h"
#include "UI/GComponent.h"
#include "Utils/ByteBuffer.h"

bool FGearBase::bDisableAllTweenEffect = false;

TSharedPtr<FGearBase> FGearBase::Create(UGObject* InOwner, EType InType)
{
    FGearBase* Gear = nullptr;
    switch (InType)
    {
    case EType::Display:
        Gear = new FGearDisplay(InOwner);
        break;
    case EType::XY:
        Gear = new FGearXY(InOwner);
        break;
    case EType::Size:
        Gear = new FGearSize(InOwner);
        break;
    case EType::Look:
        Gear = new FGearLook(InOwner);
        break;
    case EType::Color:
        Gear = new FGearColor(InOwner);
        break;
    case EType::Animation:
        Gear = new FGearAnimation(InOwner);
        break;
    case EType::Text:
        Gear = new FGearText(InOwner);
        break;
    case EType::Icon:
        Gear = new FGearIcon(InOwner);
        break;
    case EType::Display2:
        Gear = new FGearDisplay2(InOwner);
        break;
    case EType::FontSize:
        Gear = new FGearFontSize(InOwner);
        break;
    }

    return MakeShareable(Gear);
}

FGearTweenConfig::FGearTweenConfig():
    bTween(true),
    EaseType(EEaseType::QuadOut),
    Duration(0.3f),
    Delay(0),
    DisplayLockToken(0)
{
}

FGearBase::FGearBase(UGObject* InOwner) : Owner(InOwner)
{
}

FGearBase::~FGearBase()
{
}

void FGearBase::SetController(UGController* InController)
{
    if (Controller != InController)
    {
        Controller = InController;
        if (Controller != nullptr)
            Init();
    }
}

FGearTweenConfig& FGearBase::GetTweenConfig()
{
    if (!TweenConfig.IsSet())
        TweenConfig.Emplace();

    return TweenConfig.GetValue();
}

void FGearBase::Init()
{
}

void FGearBase::AddStatus(const FString& PageID, FByteBuffer* Buffer)
{
}

void FGearBase::Apply()
{
}

void FGearBase::UpdateState()
{
}

void FGearBase::UpdateFromRelations(const FVector2D& Delta)
{
}

void FGearBase::Setup(FByteBuffer* Buffer)
{
    Controller = Owner->GetParent()->GetControllerAt(Buffer->ReadShort());
    Init();

    int32 Count = Buffer->ReadShort();
    FGearDisplay* g0 = Type == EType::Display ? static_cast<FGearDisplay*>(this) : nullptr;
    FGearDisplay2* g1 = Type == EType::Display2 ? static_cast<FGearDisplay2*>(this) : nullptr;
    FGearXY* g2 = nullptr;
    if (g0)
        Buffer->ReadSArray(g0->Pages, Count);
    else if (g1)
        Buffer->ReadSArray(g1->Pages, Count);
    else
    {
        for (int32 i = 0; i < Count; i++)
        {
            const FString& page = Buffer->ReadS();
            if (page.IsEmpty())
                continue;

            AddStatus(page, Buffer);
        }

        if (Buffer->ReadBool())
            AddStatus(G_EMPTY_STRING, Buffer);
    }

    if (Buffer->ReadBool())
    {
        TweenConfig.Emplace();
        TweenConfig->bTween = true;
        TweenConfig->EaseType = (EEaseType)Buffer->ReadByte();
        TweenConfig->Duration = Buffer->ReadFloat();
        TweenConfig->Delay = Buffer->ReadFloat();
    }

    if (Buffer->Version >= 2)
    {
        g2 = Type == EType::XY ? static_cast<FGearXY*>(this) : nullptr;
        if (g2)
        {
            if (Buffer->ReadBool())
            {
                if (g2)
                {
                    g2->bPositionsInPercent = true;
                    for (int32 i = 0; i < Count; i++)
                    {
                        const FString& page = Buffer->ReadS();
                        if (page.IsEmpty())
                            continue;

                        g2->AddExtStatus(page, Buffer);
                    }

                    if (Buffer->ReadBool())
                        g2->AddExtStatus(G_EMPTY_STRING, Buffer);
                }
            }
        }
        else if (g1 != nullptr)
            g1->Condition = Buffer->ReadByte();
    }
}