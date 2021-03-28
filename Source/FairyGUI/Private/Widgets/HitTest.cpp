#include "Widgets/HitTest.h"
#include "Utils/ByteBuffer.h"
#include "UI/GObject.h"

void FPixelHitTestData::Load(FByteBuffer* Buffer)
{
    Buffer->Skip(4);
    PixelWidth = Buffer->ReadInt();
    Scale = 1.0f / Buffer->ReadByte();
    int32 PixelsLength = Buffer->ReadInt();
    Pixels.Append(Buffer->GetBuffer() + Buffer->GetPos(), PixelsLength);
}

FPixelHitTest::FPixelHitTest(const TSharedPtr<FPixelHitTestData>& InData, int32 InOffsetX, int32 InOffsetY) :
    OffsetX(InOffsetX),
    OffsetY(InOffsetY),
    Data(InData)
{
}

FPixelHitTest::~FPixelHitTest()
{

}

bool FPixelHitTest::HitTest(const FBox2D& ContentRect, const FVector2D& LayoutScaleMultiplier, const FVector2D& LocalPoint) const
{
    int32 x = FMath::FloorToInt((LocalPoint.X / LayoutScaleMultiplier.X - OffsetX) * Data->Scale);
    int32 y = FMath::FloorToInt((LocalPoint.Y / LayoutScaleMultiplier.Y - OffsetY) * Data->Scale);
    if (x < 0 || y < 0 || x >= Data->PixelWidth)
        return false;

    int32 pos = y * Data->PixelWidth + x;
    int32 pos2 = pos / 8;
    int32 pos3 = pos % 8;

    if (pos2 >= 0 && pos2 < Data->Pixels.Num())
        return ((Data->Pixels[pos2] >> pos3) & 0x1) > 0;
    else
        return false;
}

FChildHitTest::FChildHitTest(UGObject* InObj) :Obj(InObj)
{
}

FChildHitTest::~FChildHitTest()
{
}

bool FChildHitTest::HitTest(const FBox2D& ContentRect, const FVector2D& LayoutScaleMultiplier, const FVector2D& LocalPoint) const
{
    if (!Obj.IsValid() || Obj->GetParent() == nullptr)
        return false;

    IHitTest* HitArea = Obj->GetHitArea();
    if (HitArea == nullptr)
        return false;

    FVector2D NewPoint = LocalPoint + Obj->GetPosition();
    FBox2D NewRect = FBox2D(FVector2D::ZeroVector, Obj->GetSize());
    if (!NewRect.IsInside(NewPoint))
        return false;

    FVector2D NewMultiplier = Obj->GetSize() / Obj->SourceSize;
    if (NewMultiplier.ContainsNaN())
        NewMultiplier.Set(1, 1);

    return HitArea->HitTest(NewRect, NewMultiplier, NewPoint);
}