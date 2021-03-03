#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UIPackageAsset.generated.h"

UCLASS()
class FAIRYGUI_API UUIPackageAsset : public UObject
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere)
    TArray<uint8> Data;

#if WITH_EDITORONLY_DATA
    UPROPERTY(Instanced)
    class UAssetImportData* AssetImportData;

    virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
#endif
};