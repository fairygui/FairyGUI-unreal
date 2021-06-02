#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UIPackage.generated.h"

class FPackageItem;
class UGObject;
class FByteBuffer;
class UUIPackageAsset;

UCLASS(BlueprintType)
class FAIRYGUI_API UUIPackage : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    static const FString& GetBranch();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    static void SetBranch(const FString& InBranch);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI", meta = (DisplayName = "Get UI Global Variable"))
    static FString GetVar(const FString& VarKey);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI", meta = (DisplayName = "Set UI Global Variable"))
    static void SetVar(const FString& VarKey, const FString& VarValue);

    static UUIPackage* AddPackage(const TCHAR* InAssetPath, UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI", meta = (WorldContext = "WorldContextObject"))
    static UUIPackage* AddPackage(class UUIPackageAsset* InAsset, UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI", meta = (WorldContext = "WorldContextObject"))
    static void RemovePackage(const FString& IDOrName, UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    static void RemoveAllPackages();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    static UUIPackage* GetPackageByID(const FString& PackageID);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    static UUIPackage* GetPackageByName(const FString& PackageName);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI", meta = (DisplayName = "Create UI", DeterminesOutputType = "ClassType", WorldContext = "WorldContextObject"))
    static UGObject* CreateObject(const FString& PackageName, const FString& ResourceName, UObject* WorldContextObject, TSubclassOf<UGObject> ClassType = nullptr);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI", meta = (DisplayName = "Create UI From URL", DeterminesOutputType = "ClassType", WorldContext = "WorldContextObject"))
    static UGObject* CreateObjectFromURL(const FString& URL, UObject* WorldContextObject, TSubclassOf<UGObject> ClassType = nullptr);

    static FString GetItemURL(const FString& PackageName, const FString& ResourceName);
    static TSharedPtr<FPackageItem> GetItemByURL(const FString& URL);
    static FString NormalizeURL(const FString& URL);

    static int32 Constructing;

public:
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    static void RegisterFont(const FString& FontFace, UObject* Font);

public:
    UUIPackage();
    virtual  ~UUIPackage();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    const FString& GetID() const { return ID; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    const FString& GetName() const { return Name; }

    TSharedPtr<FPackageItem> GetItem(const FString& ResourceID) const;
    TSharedPtr<FPackageItem> GetItemByName(const FString& ResourceName);
    void* GetItemAsset(const TSharedPtr<FPackageItem>& Item);

    UGObject* CreateObject(const FString& ResourceName, UObject* WorldContextObject);
    UGObject* CreateObject(const TSharedPtr<FPackageItem>& Item, UObject* WorldContextObject);

private:
    void Load(FByteBuffer* Buffer);
    void LoadAtlas(const TSharedPtr<FPackageItem>& Item);
    void LoadImage(const TSharedPtr<FPackageItem>& Item);
    void LoadMovieClip(const TSharedPtr<FPackageItem>& Item);
    void LoadFont(const TSharedPtr<FPackageItem>& Item);
    void LoadSound(const TSharedPtr<FPackageItem>& Item);

private:
    FString ID;
    FString Name;
    FString AssetPath;
    UPROPERTY(Transient)
    UUIPackageAsset* Asset;

    TArray<TSharedPtr<FPackageItem>> Items;
    TMap<FString, TSharedPtr<FPackageItem>> ItemsByID;
    TMap<FString, TSharedPtr<FPackageItem>> ItemsByName;
    TMap<FString, struct FAtlasSprite*> Sprites;
    FString CustomID;
    TArray<TMap<FString, FString>> Dependencies;
    TArray<FString> Branches;
    int32 BranchIndex;
    TSet<uint32> RefWorlds;

    friend class FPackageItem;
    friend class UFairyApplication;
};

UCLASS(Transient)
class FAIRYGUI_API UUIPackageStatic : public UObject
{
    GENERATED_BODY()

public:
    static UUIPackageStatic* Singleton;
    static UUIPackageStatic& Get();
    static void Destroy();

    UPROPERTY(Transient)
    TArray<UUIPackage*> PackageList;
    TMap<FString, UUIPackage*> PackageInstByID;
    TMap<FString, UUIPackage*> PackageInstByName;
    TMap<FString, FString> Vars;
    FString Branch;
    UPROPERTY(Transient)
    TMap<FString, UObject*> Fonts;
};