#include "UI/ControllerAction/ControllerAction.h"
#include "UI/ControllerAction/ChangePageAction.h"
#include "UI/ControllerAction/PlayTransitionAction.h"
#include "Utils/ByteBuffer.h"

FControllerAction * FControllerAction::CreateAction(int32 ActionType)
{
    switch (ActionType)
    {
    case 0:
        return new FPlayTransitionAction();

    case 1:
        return new FChangePageAction();
    }
    return nullptr;
}

FControllerAction::FControllerAction()
{
}

FControllerAction::~FControllerAction()
{
}

void FControllerAction::Run(UGController* Controller, const FString& PreviousPage, const FString& CurrentPage)
{
    if ((FromPage.Num() == 0 || FromPage.Contains(PreviousPage))
        && (ToPage.Num() == 0 || ToPage.Contains(CurrentPage)))
        Enter(Controller);
    else
        Leave(Controller);
}

void FControllerAction::Setup(FByteBuffer * Buffer)
{
    int32 cnt;

    cnt = Buffer->ReadShort();
    FromPage.SetNum(cnt);
    for (int32 i = 0; i < cnt; i++)
        FromPage[i] = Buffer->ReadS();

    cnt = Buffer->ReadShort();
    ToPage.SetNum(cnt);
    for (int32 i = 0; i < cnt; i++)
        ToPage[i] = Buffer->ReadS();
}
