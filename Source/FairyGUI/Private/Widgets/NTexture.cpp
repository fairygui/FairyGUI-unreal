#include "Widgets/NTexture.h"
#include "FairyApplication.h"

UNTexture* UNTexture::WhiteTexture = nullptr;

UNTexture* UNTexture::GetWhiteTexture()
{
    if (WhiteTexture == nullptr)
    {
        UTexture2D* NativeTexture = UTexture2D::CreateTransient(2, 2);
        uint8* MipData = (uint8*)NativeTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
        for (int32 i = 0; i < 16; i++)
            *(MipData + i) = 255;
        NativeTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
#if WITH_EDITORONLY_DATA
        NativeTexture->CompressionNone = true;
        NativeTexture->MipGenSettings = TMGS_NoMipmaps;
#endif // WITH_EDITORONLY_DATA
        NativeTexture->CompressionSettings = TC_Default;
        NativeTexture->UpdateResource();

        WhiteTexture = NewObject<UNTexture>();
        WhiteTexture->AddToRoot();
        WhiteTexture->Init(NativeTexture);
    }

    return WhiteTexture;
}

void UNTexture::DestroyWhiteTexture()
{
    if (WhiteTexture != nullptr)
    {
        WhiteTexture->RemoveFromRoot();
        WhiteTexture = nullptr;
    }
}

UNTexture::UNTexture() :
    UVRect(ForceInit),
    Region(ForceInit),
    Offset(ForceInit),
    OriginalSize(ForceInit)
{

}

void UNTexture::Init(UTexture2D* InNativeTexture)
{
    Init(InNativeTexture, 1, 1);
}

void UNTexture::Init(UTexture2D* InNativeTexture, float ScaleX, float ScaleY)
{
    NativeTexture = InNativeTexture;
    UVRect = FBox2D(FVector2D::ZeroVector, FVector2D(ScaleX, ScaleY));
    if (ScaleY < 0)
    {
        UVRect.Min.Y = -ScaleY;
        UVRect.Max.Y = 0;
    }
    if (ScaleX < 0)
    {
        UVRect.Min.X = -ScaleX;
        UVRect.Max.X = 0;
    }
    if (NativeTexture != nullptr)
        OriginalSize.Set(NativeTexture->GetSurfaceWidth(), NativeTexture->GetSurfaceHeight());
    Region = FBox2D(FVector2D::ZeroVector, FVector2D(OriginalSize.X, OriginalSize.Y));
}

void UNTexture::Init(UTexture2D* InNativeTexture, const FBox2D& InRegion)
{
    NativeTexture = InNativeTexture;
    Region = InRegion;
    Region.bIsValid = true;
    OriginalSize = Region.GetSize();

    if (NativeTexture != nullptr)
    {
        UVRect = FBox2D(FVector2D(Region.Min.X / NativeTexture->GetSurfaceWidth(),
            Region.Min.Y / NativeTexture->GetSurfaceHeight()),
            FVector2D(Region.Max.X / NativeTexture->GetSurfaceWidth(),
                Region.Max.Y / NativeTexture->GetSurfaceHeight()));
    }
    else
        UVRect = FBox2D(FVector2D::ZeroVector, FVector2D(1, 1));
}

void UNTexture::Init(UNTexture* InRoot, const FBox2D& InRegion, bool bInRotated)
{
    Root = InRoot;
    NativeTexture = Root->NativeTexture;
    bRotated = bInRotated;
    Region = InRegion;
    Region.bIsValid = true;

    Region.Min.X += Root->Region.Min.X;
    Region.Min.Y += Root->Region.Min.Y;
    FVector2D RootSize = Root->GetSize();
    UVRect = FBox2D(FVector2D(Region.Min.X * Root->UVRect.GetSize().X / RootSize.X,
        Region.Min.Y * Root->UVRect.GetSize().Y / RootSize.Y),
        FVector2D(Region.Max.X * Root->UVRect.GetSize().X / RootSize.X,
            Region.Max.Y * Root->UVRect.GetSize().Y / RootSize.Y));

    if (bRotated)
    {
        FVector2D TmpSize = Region.GetSize();
        Region.Max.X = Region.Min.X + TmpSize.Y;
        Region.Max.Y = Region.Min.Y + TmpSize.X;

        TmpSize = UVRect.GetSize();
        UVRect.Max.X = UVRect.Min.X + TmpSize.Y;
        UVRect.Max.Y = UVRect.Min.Y + TmpSize.X;
    }
    OriginalSize = Region.GetSize();
}

void UNTexture::Init(UNTexture* InRoot, const FBox2D& InRegion, bool bInRotated, const FVector2D& InOriginalSize, const FVector2D& InOffset)
{
    Init(InRoot, InRegion, bInRotated);

    OriginalSize = InOriginalSize;
    Offset = InOffset;
}

FVector2D UNTexture::GetSize() const
{
    return Region.GetSize();
}

FBox2D UNTexture::GetDrawRect(FBox2D& InDrawRect) const
{
    if (OriginalSize == Region.GetSize())
        return InDrawRect;

    FVector2D Scale = InDrawRect.GetSize() / OriginalSize;
    return FBox2D(Offset * Scale, (Region.GetSize() + Offset)*Scale);
}