#pragma once

#include "CoreMinimal.h"

class UGController;
class FByteBuffer;

class FControllerAction
{
public:
    static FControllerAction* CreateAction(int32 ActionType);

    FControllerAction();
    virtual ~FControllerAction();

    void Run(UGController* Controller, const FString& PreviousPage, const FString& CurrentPage);
    virtual void Setup(FByteBuffer * Buffer);

    TArray<FString> FromPage;
    TArray<FString> ToPage;

protected:
    virtual void Enter(UGController* Controller) = 0;
    virtual void Leave(UGController* Controller) = 0;
};