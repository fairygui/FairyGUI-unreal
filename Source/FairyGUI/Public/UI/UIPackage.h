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
    static const FString& GetBranch() { return Branch; }
    static void SetBranch(const FString& InBranch);

    static FString GetVar(const FString& VarKey);
    static void SetVar(const FString& VarKey, const FString& VarValue);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    static UUIPackage* AddPackage(class UUIPackageAsset* InAsset);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    static void RemovePackage(const FString& IDOrName);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    static void RemoveAllPackages();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    static UUIPackage* GetPackageByName(const FString& PackageName);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI", meta = (DisplayName = "Create UI", DeterminesOutputType = "ClassType"))
    static UGObject* CreateObject(const FString& PackageName, const FString& ResourceName, TSubclassOf<UGObject> ClassType = nullptr);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI", meta = (DisplayName="Create UI From URL", DeterminesOutputType = "ClassType"))
    static UGObject* CreateObjectFromURL(const FString& URL, TSubclassOf<UGObject> ClassType = nullptr);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    static UGWindow* CreateWindow(const FString& PackageName, const FString& ResourceName);

    static UUIPackage* GetPackageByID(const FString& PackageID);
    static FString GetItemURL(const FString& PackageName, const FString& ResourceName);
    static TSharedPtr<FPackageItem> GetItemByURL(const FString& URL);
    static FString NormalizeURL(const FString& URL);

    static int32 Constructing;

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

    UGObject* CreateObject(const FString& ResourceName);
    UGObject* CreateObject(const TSharedPtr<FPackageItem>& Item);

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

    static TMap<FString, UUIPackage*> PackageInstByID;
    static TMap<FString, UUIPackage*> PackageInstByName;
    static TMap<FString, FString> Vars;
    static FString Branch;

    friend class FPackageItem;
    friend class UFairyApplication;
};
