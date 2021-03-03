#pragma once

#include "GList.h"
#include "GTree.generated.h"

UCLASS(BlueprintType)
class FAIRYGUI_API UGTree : public UGList
{
    GENERATED_BODY()

public:
    UGTree();
    virtual ~UGTree();
};