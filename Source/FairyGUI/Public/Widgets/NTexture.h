#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Runtime/Engine/Classes/Engine/Texture2D.h"
#include "NTexture.generated.h"

UCLASS()
class FAIRYGUI_API UNTexture : public UObject
{
    GENERATED_BODY()

public:
    static UNTexture* GetWhiteTexture();
    static void DestroyWhiteTexture();

    UNTexture();

    UPROPERTY(Transient)
    UNTexture* Root;
    UPROPERTY(Transient)
    UTexture2D* NativeTexture;

    FBox2D UVRect;
    bool bRotated;
    FBox2D Region;
    FVector2D Offset;
    FVector2D OriginalSize;

    void Init(UTexture2D* NewNativeTexture);
    void Init(UTexture2D* NewNativeTexture, float ScaleX, float ScaleY);
    void Init(UTexture2D* NewNativeTexture, const FBox2D& NewRegion);
    void Init(UNTexture* NewRoot, const FBox2D& NewRegion, bool bNewRotated);
    void Init(UNTexture* NewRoot, const FBox2D& NewRegion, bool bNewRotated, const FVector2D& NewOriginalSize, const FVector2D& NewOffset);

    FVector2D GetSize() const;
    FBox2D GetDrawRect(FBox2D& InDrawRect) const;

private:
    static UNTexture* WhiteTexture;

    friend class UFairyApplication;
};