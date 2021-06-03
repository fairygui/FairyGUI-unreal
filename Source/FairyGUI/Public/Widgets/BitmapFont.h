#pragma once

#include "CoreMinimal.h"

class UNTexture;

struct FAIRYGUI_API FBitmapFont : public FGCObject
{
    struct FGlyph
    {
        FVector2D Offset;
        FVector2D Size;
        float XAdvance;
        float LineHeight;
        FBox2D UVRect;
        int32 Channel;
    };

    int32 FontSize;
    bool bResizable;
    bool bHasChannel;
    bool bCanTint;

    TMap<TCHAR, FGlyph> Glyphs;

    UNTexture* Texture;

    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
};