#include "UI/ControllerAction/PlayTransitionAction.h"
#include "UI/GController.h"
#include "UI/Transition.h"
#include "UI/GComponent.h"
#include "Utils/ByteBuffer.h"

FPlayTransitionAction::FPlayTransitionAction() :
    PlayTimes(1),
    Delay(0),
    bStopOnExit(false),
    CurrentTransition(nullptr)
{
}

void FPlayTransitionAction::Setup(FByteBuffer* Buffer)
{
    FControllerAction::Setup(Buffer);

    TransitionName = Buffer->ReadS();
    PlayTimes = Buffer->ReadInt();
    Delay = Buffer->ReadFloat();
    bStopOnExit = Buffer->ReadBool();
}

void FPlayTransitionAction::Enter(UGController* Controller)
{
    UTransition* trans = Cast<UGComponent>(Controller->GetOuter())->GetTransition(TransitionName);
    if (trans != nullptr)
    {
        if (CurrentTransition != nullptr && CurrentTransition->IsPlaying())
            trans->ChangePlayTimes(PlayTimes);
        else
            trans->Play(PlayTimes, Delay);
        CurrentTransition = trans;
    }
}

void FPlayTransitionAction::Leave(UGController* Controller)
{
    if (bStopOnExit && CurrentTransition != nullptr)
    {
        CurrentTransition->Stop();
        CurrentTransition = nullptr;
    }
}