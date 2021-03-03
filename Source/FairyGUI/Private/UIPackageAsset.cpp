#include "UIPackageAsset.h"
#include "EditorFramework/AssetImportData.h"

void UUIPackageAsset::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
#if WITH_EDITORONLY_DATA
    if (AssetImportData)
    {
        OutTags.Add(FAssetRegistryTag(SourceFileTagName(),
            AssetImportData->GetSourceData().ToJson(),
            FAssetRegistryTag::TT_Hidden));
#if WITH_EDITOR
        AssetImportData->AppendAssetRegistryTags(OutTags);
#endif
    }
#endif

    Super::GetAssetRegistryTags(OutTags);
}