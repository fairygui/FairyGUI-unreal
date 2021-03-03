#pragma once

#include "GTextField.h"
#include "GRichTextField.generated.h"

UCLASS(BlueprintType)
class FAIRYGUI_API UGRichTextField : public UGTextField
{
    GENERATED_BODY()

public:
    UGRichTextField();
    virtual ~UGRichTextField();

    UPROPERTY(BlueprintAssignable, Category = "FairyGUI|Event")
    FGUIEventDynMDelegate OnClickLink;

protected:
};