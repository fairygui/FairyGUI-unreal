#include "FairyCommons.h"
#include "Framework/Application/SlateApplication.h"

DEFINE_LOG_CATEGORY(LogFairyGUI);

const FString G_EMPTY_STRING("");

template< class UserClass, typename... VarTypes >
void DelayCall(FTimerHandle &InOutHandle, UserClass* InUserObject, typename TMemFunPtrType<false, UserClass, void(VarTypes...)>::Type inTimerMethod, VarTypes... Vars)
{
    if (!GWorld->GetTimerManager().TimerExists(InOutHandle))
        InOutHandle = GWorld->GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(InUserObject, inTimerMethod, Vars...));
}

void CancelDelayCall(FTimerHandle& InHandle)
{
    GWorld->GetTimerManager().ClearTimer(InHandle);
}