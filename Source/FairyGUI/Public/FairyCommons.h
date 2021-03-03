#pragma once

#include "CoreMinimal.h"

FAIRYGUI_API DECLARE_LOG_CATEGORY_EXTERN(LogFairyGUI, Log, All)

extern const FString FAIRYGUI_API G_EMPTY_STRING;

DECLARE_DELEGATE_RetVal(class UGComponent*, FGComponentCreator);

template< class UserClass, typename... VarTypes >
void DelayCall(FTimerHandle& InOutHandle, UserClass* InUserObject, typename TMemFunPtrType<false, UserClass, void(VarTypes...)>::Type inTimerMethod, VarTypes...);
void CancelDelayCall(FTimerHandle& InHandle);