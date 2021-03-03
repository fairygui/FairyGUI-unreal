#include "FairyGUIFactory.h"
#include "Serialization/BufferArchive.h"
#include "EditorFramework/AssetImportData.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "UIPackageAsset.h"

UFairyGUIFactory::UFairyGUIFactory()
{
    SupportedClass = UUIPackageAsset::StaticClass();
    bEditorImport = true;
    bCreateNew = false;
    bText = false;
    Formats.Add(TEXT("fui;FairyGUI package files"));
}

UObject* UFairyGUIFactory::FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn)
{
    UUIPackageAsset* UIAsset = NewObject<UUIPackageAsset>(InParent, InName, Flags);

    const int32 InDataSize = BufferEnd - Buffer;
    UIAsset->Data.Empty(InDataSize);
    UIAsset->Data.AddUninitialized(InDataSize);
    FMemory::Memcpy(UIAsset->Data.GetData(), Buffer, InDataSize);

    if (!UIAsset->AssetImportData)
    {
        UIAsset->AssetImportData = NewObject<UAssetImportData>(UIAsset, UAssetImportData::StaticClass());
    }
    UIAsset->AssetImportData->Update(CurrentFilename);

    return UIAsset;
}

bool UFairyGUIFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
    UUIPackageAsset* UIAsset = Cast<UUIPackageAsset>(Obj);
    if (UIAsset && UIAsset->AssetImportData)
    {
        UIAsset->AssetImportData->ExtractFilenames(OutFilenames);
        return true;
    }
    return false;
}

void UFairyGUIFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
    UUIPackageAsset* UIAsset = Cast<UUIPackageAsset>(Obj);
    if (UIAsset && ensure(NewReimportPaths.Num() == 1))
    {
        UIAsset->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
    }
}

EReimportResult::Type UFairyGUIFactory::Reimport(UObject* Obj)
{
    UUIPackageAsset* UIAsset = Cast<UUIPackageAsset>(Obj);
    if (!UIAsset)
    {
        return EReimportResult::Failed;
    }

    const FString Filename = UIAsset->AssetImportData->GetFirstFilename();

    if (!Filename.Len() || IFileManager::Get().FileSize(*Filename) == INDEX_NONE)
    {
        return EReimportResult::Failed;
    }

    if (UFactory::StaticImportObject(
        UIAsset->GetClass(),
        UIAsset->GetOuter(),
        *UIAsset->GetName(),
        RF_Public | RF_Standalone,
        *Filename,
        NULL,
        this))
    {
        if (UIAsset->GetOuter())
        {
            UIAsset->GetOuter()->MarkPackageDirty();
        }
        else
        {
            UIAsset->MarkPackageDirty();
        }

        return EReimportResult::Succeeded;
    }
    return EReimportResult::Failed;
}