#pragma once

#include "CoreMinimal.h"
#include "FieldTypes.h"
#include "PackageItem.h"

class UGComponent;
class UGLoader;
class UGObject;

DECLARE_DELEGATE_RetVal(UGLoader*, FGLoaderCreator);

class FAIRYGUI_API FUIObjectFactory
{
public:
    static void SetExtension(const FString& URL, FGComponentCreator Creator);
    static void SetExtension(const FString& URL, TSubclassOf<UGComponent> ClassType);
    static UGObject* NewObject(const TSharedPtr<FPackageItem>& PackageItem);
    static UGObject* NewObject(EObjectType Type);

    static FGLoaderCreator LoaderCreator;
    static void ResolvePackageItemExtension(const TSharedPtr<FPackageItem>& PackageItem);

public:
    static TMap<FString, FGComponentCreator> PackageItemExtensions;

    friend class UFGUIPackage;
};