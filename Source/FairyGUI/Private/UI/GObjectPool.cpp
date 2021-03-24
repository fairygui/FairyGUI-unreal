#include "UI/GObjectPool.h"
#include "UI/GObject.h"
#include "UI/UIPackage.h"

UGObject* FGObjectPool::GetObject(const FString & URL, UObject* WorldContextObject)
{
    FString URL2 = UUIPackage::NormalizeURL(URL);
    if (URL2.Len() == 0)
        return nullptr;

    UGObject* ret;
    TArray<UGObject*>& arr = Pool.FindOrAdd(URL2);
    if (arr.Num() > 0)
        ret = arr.Pop();
    else
        ret = UUIPackage::CreateObjectFromURL(URL2, WorldContextObject);
    return ret;
}

void FGObjectPool::ReturnObject(UGObject* Obj)
{
    TArray<UGObject*>& arr = Pool.FindOrAdd(Obj->GetResourceURL());
    arr.Add(Obj);
}

void FGObjectPool::AddReferencedObjects(FReferenceCollector& Collector)
{
    for (auto& Elem : Pool)
    {
        Collector.AddReferencedObjects(Elem.Value);
    }
}