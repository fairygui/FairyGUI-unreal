#pragma once

#include "CoreMinimal.h"
#include "FieldTypes.h"
#include "PackageItem.h"

class UGComponent;
class UGLoader;
class UGObject;

class FAIRYGUI_API FUIObjectFactory
{
public:
    static void SetExtension(const FString& URL, FGComponentCreator Creator);
    static void SetExtension(const FString& URL, TSubclassOf<UGComponent> ClassType);

    static UGObject* NewObject(const TSharedPtr<FPackageItem>& PackageItem, UObject* Outer);
    static UGObject* NewObject(EObjectType Type, UObject* Outer);

    static void ResolvePackageItemExtension(const TSharedPtr<FPackageItem>& PackageItem);

public:
    static TMap<FString, FGComponentCreator> PackageItemExtensions;
    static TSubclassOf<class UGLoader> LoaderExtension;

    friend class UFGUIPackage;
};