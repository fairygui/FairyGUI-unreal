#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "FieldTypes.h"
#include "Engine.h"
#include "FairyCommons.h"
#include "Widgets/HitTest.h"

class FByteBuffer;
struct FMovieClipData;
struct FBitmapFont;

class UUIPackage;
class UNTexture;
class UGComponent;

class FAIRYGUI_API FPackageItem : public FGCObject, public TSharedFromThis<FPackageItem>
{
public:
    FPackageItem();

    void Load();
    TSharedPtr<FPackageItem> GetBranch();
    TSharedPtr<FPackageItem> GetHighResolution();

    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

public:
    UUIPackage* Owner;

    EPackageItemType Type;
    EObjectType ObjectType;
    FString ID;
    FString Name;
    FVector2D Size;
    FString File;
    TSharedPtr<FByteBuffer> RawData;
    TOptional<TArray<FString>> Branches;
    TOptional<TArray<FString>> HighResolution;

    //atlas
    UNTexture* Texture;

    //image
    TOptional<FBox2D> Scale9Grid;
    bool bScaleByTile;
    int32 TileGridIndice;
    TSharedPtr<FPixelHitTestData> PixelHitTestData;

    //movie clip
    TSharedPtr<FMovieClipData> MovieClipData;

    //component
    FGComponentCreator ExtensionCreator;
    bool bTranslated;

    //font
    TSharedPtr<FBitmapFont> BitmapFont;

    //sound
    TSharedPtr<FSlateSound> Sound;
};
