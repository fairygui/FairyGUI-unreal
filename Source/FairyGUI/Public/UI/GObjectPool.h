#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

class UGObject;

class FGObjectPool : public FGCObject
{
public:
    UGObject* GetObject(const FString& URL, UObject* WorldContextObject);
    void ReturnObject(UGObject* Obj);

    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

    virtual FString GetReferencerName() const override
    {
        return TEXT("FGObjectPool");
    };

private:
    TMap<FString, TArray<UGObject*>> Pool;
};