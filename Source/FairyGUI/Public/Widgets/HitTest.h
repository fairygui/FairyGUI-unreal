#pragma once

#include "CoreMinimal.h"

class FAIRYGUI_API IHitTest
{
public:
    virtual bool HitTest(const FBox2D& ContentRect, const FVector2D& LayoutScaleMultiplier, const FVector2D& LocalPoint) const = 0;
};

struct FAIRYGUI_API FPixelHitTestData
{
public:
    int32 PixelWidth;
    float Scale;
    TArray<uint8> Pixels;

    void Load(class FByteBuffer* Buffer);
};

class FAIRYGUI_API FPixelHitTest : public IHitTest
{
public:
    FPixelHitTest(const TSharedPtr<FPixelHitTestData>& Data, int32 OffsetX, int32 OffsetY);
    virtual ~FPixelHitTest();

    bool HitTest(const FBox2D& ContentRect, const FVector2D& LayoutScaleMultiplier, const FVector2D& LocalPoint) const;

    int32 OffsetX;
    int32 OffsetY;

private:
    TSharedPtr<FPixelHitTestData> Data;
};

class UGObject;
class FAIRYGUI_API FChildHitTest : public IHitTest
{
public:
    FChildHitTest(UGObject* InObj);
	virtual ~FChildHitTest();

    bool HitTest(const FBox2D& ContentRect, const FVector2D& LayoutScaleMultiplier, const FVector2D& LocalPoint) const;

    TWeakObjectPtr<UGObject> Obj;
};