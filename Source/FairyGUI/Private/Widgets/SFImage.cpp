#include "Widgets/SFImage.h"
#include "Widgets/NTexture.h"
#include "Widgets/Mesh/VertexHelper.h"
#include "UI/FieldTypes.h"

SFImage::SFImage() :
    bScaleByTile(false),
    TextureScale(1, 1),
    TileGridIndice(0)
{
    Graphics.SetMeshFactory(MakeShared<FMeshFactory>(this));
}

void SFImage::Construct(const FArguments& InArgs)
{
    SDisplayObject::Construct(SDisplayObject::FArguments().GObject(InArgs._GObject));
}

void SFImage::SetTexture(UNTexture* InTexture)
{
    Graphics.SetTexture(InTexture);

    if (InTexture != nullptr && Size.IsZero())
    {
        SetSize(InTexture->GetSize());
        Invalidate(EInvalidateWidget::LayoutAndVolatility);
    }
}

void SFImage::SetNativeSize()
{
    if (Graphics.GetTexture() != nullptr)
        SetSize(Graphics.GetTexture()->GetSize());
}

void SFImage::SetScale9Grid(const TOptional<FBox2D>& InGridRect)
{
    Scale9Grid = InGridRect;
}

void SFImage::SetScaleByTile(bool bInScaleByTile)
{
    if (bScaleByTile != bInScaleByTile)
    {
        bScaleByTile = bInScaleByTile;
        Graphics.SetMeshDirty();
    }
}

void SFImage::SetTileGridIndice(int32 InTileGridIndex)
{
    if (TileGridIndice != InTileGridIndex)
    {
        TileGridIndice = InTileGridIndex;
        Graphics.SetMeshDirty();
    }
}

EFillMethod SFImage::GetFillMethod() const
{
    return FillMesh.IsValid() ? FillMesh->Method : EFillMethod::None;
}

void SFImage::SetFillMethod(EFillMethod InMethod)
{
    if (!FillMesh.IsValid())
    {
        if (InMethod == EFillMethod::None)
            return;

        FillMesh = MakeUnique<FFillMesh>();
    }

    if (FillMesh->Method != InMethod)
    {
        FillMesh->Method = InMethod;
        Graphics.SetMeshDirty();
    }
}

int32 SFImage::GetFillOrigin() const
{
    return FillMesh.IsValid() ? FillMesh->Origin : 0;
}

void SFImage::SetFillOrigin(int32 InOrigin)
{
    if (!FillMesh.IsValid())
        FillMesh = MakeUnique<FFillMesh>();

    if (FillMesh->Origin != InOrigin)
    {
        FillMesh->Origin = InOrigin;
        Graphics.SetMeshDirty();
    }
}

bool SFImage::IsFillClockwise() const
{
    return FillMesh.IsValid() ? FillMesh->bClockwise : true;
}

void SFImage::SetFillClockwise(bool bInClockwise)
{
    if (!FillMesh.IsValid())
        FillMesh = MakeUnique<FFillMesh>();

    if (FillMesh->bClockwise != bInClockwise)
    {
        FillMesh->bClockwise = bInClockwise;
        Graphics.SetMeshDirty();
    }
}

float SFImage::GetFillAmount() const
{
    return FillMesh.IsValid() ? FillMesh->Amount : 0;
}

void SFImage::SetFillAmount(float InAmount)
{
    if (!FillMesh.IsValid())
        FillMesh = MakeUnique<FFillMesh>();

    if (FillMesh->Amount != InAmount)
    {
        FillMesh->Amount = InAmount;
        Graphics.SetMeshDirty();
    }
}

int32 SFImage::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    const bool bIsEnabled = ShouldBeEnabled(bParentEnabled);
    const_cast<SFImage*>(this)->Graphics.Paint(AllottedGeometry, OutDrawElements, LayerId,
        InWidgetStyle.GetColorAndOpacityTint().A, bIsEnabled);
    return LayerId;
}

void SFImage::OnPopulateMesh(FVertexHelper& Helper)
{
    if (FillMesh.IsValid() && FillMesh->Method != EFillMethod::None)
    {
        FillMesh->OnPopulateMesh(Helper);
    }
    else if (bScaleByTile)
    {
        UNTexture* Texture = Graphics.GetTexture();
        if (Texture->Root == Texture
            && Texture->NativeTexture != nullptr
            && Texture->NativeTexture->AddressX == TextureAddress::TA_Mirror
            && Texture->NativeTexture->AddressY == TextureAddress::TA_Mirror)
        {
            FBox2D UVRect = Helper.UVRect;
            UVRect.Max = UVRect.Min + UVRect.GetSize() * Helper.ContentRect.GetSize() / Texture->GetSize() * TextureScale;

            Helper.AddQuad(Helper.ContentRect, Helper.VertexColor, UVRect);
            Helper.AddTriangles();
        }
        else
        {
            FBox2D ContentRect = Helper.ContentRect;
            ContentRect.Max = ContentRect.Min + ContentRect.GetSize() * TextureScale;

            TileFill(Helper, ContentRect, Helper.UVRect, Graphics.GetTexture()->GetSize());
            Helper.AddTriangles();
        }
    }
    else if (Scale9Grid.IsSet())
    {
        SliceFill(Helper);
    }
    else
        Graphics.PopulateDefaultMesh(Helper);
}

void SFImage::SliceFill(FVertexHelper& Helper)
{
    const SlateIndex TRIANGLES_9_GRID[] = {
        4, 0, 1, 1, 5, 4,
        5, 1, 2, 2, 6, 5,
        6, 2, 3, 3, 7, 6,
        8, 4, 5, 5, 9, 8,
        9, 5, 6, 6, 10, 9,
        10, 6, 7, 7, 11, 10,
        12, 8, 9, 9, 13, 12,
        13, 9, 10, 10, 14, 13,
        14, 10, 11,
        11, 15, 14
    };

    const int32 gridTileIndice[] = { -1, 0, -1, 2, 4, 3, -1, 1, -1 };

    static float gridX[4];
    static float gridY[4];
    static float gridTexX[4];
    static float gridTexY[4];

    UNTexture* Texture = Graphics.GetTexture();
    FBox2D GridRect = Scale9Grid.GetValue();
    FBox2D ContentRect = Helper.ContentRect;
    ContentRect.Max = ContentRect.Min + ContentRect.GetSize() * TextureScale;
    FBox2D UVRect = Helper.UVRect;
    FVector2D TextureSize = Texture->GetSize();
    EFlipType FlipType = Graphics.GetFlip();

    if (FlipType != EFlipType::None)
    {
        if (FlipType == EFlipType::Horizontal || FlipType == EFlipType::Both)
        {
            GridRect.Min.X = TextureSize.X - GridRect.Max.X;
            GridRect.Max.X = GridRect.Min.X + GridRect.GetSize().X;
        }

        if (FlipType == EFlipType::Vertical || FlipType == EFlipType::Both)
        {
            GridRect.Min.Y = TextureSize.Y - GridRect.Max.Y;
            GridRect.Max.Y = GridRect.Min.Y + GridRect.GetSize().Y;
        }
    }

    FVector2D Scale = UVRect.GetSize() / TextureSize;

    gridTexX[0] = UVRect.Min.X;
    gridTexX[1] = UVRect.Min.X + GridRect.Min.X * Scale.X;
    gridTexX[2] = UVRect.Min.X + GridRect.Max.X * Scale.X;
    gridTexX[3] = UVRect.Max.X;
    gridTexY[0] = UVRect.Min.Y;
    gridTexY[1] = UVRect.Min.Y + GridRect.Min.Y * Scale.Y;
    gridTexY[2] = UVRect.Min.Y + GridRect.Max.Y * Scale.Y;
    gridTexY[3] = UVRect.Max.Y;

    if (ContentRect.GetSize().X >= (Scale.X - GridRect.GetSize().X))
    {
        gridX[1] = GridRect.Min.X;
        gridX[2] = ContentRect.GetSize().X - (TextureSize.X - GridRect.Max.X);
        gridX[3] = ContentRect.GetSize().X;
    }
    else
    {
        float tmp = GridRect.Min.X / (TextureSize.X - GridRect.Max.X);
        tmp = ContentRect.GetSize().X * tmp / (1 + tmp);
        gridX[1] = tmp;
        gridX[2] = tmp;
        gridX[3] = ContentRect.GetSize().X;
    }

    if (ContentRect.GetSize().Y >= (TextureSize.Y - GridRect.GetSize().Y))
    {
        gridY[1] = GridRect.Min.Y;
        gridY[2] = ContentRect.GetSize().Y - (TextureSize.Y - GridRect.Max.Y);
        gridY[3] = ContentRect.GetSize().Y;
    }
    else
    {
        float tmp = GridRect.Min.Y / (TextureSize.Y - GridRect.Max.Y);
        tmp = ContentRect.GetSize().Y * tmp / (1 + tmp);
        gridY[1] = tmp;
        gridY[2] = tmp;
        gridY[3] = ContentRect.GetSize().Y;
    }

    if (TileGridIndice == 0)
    {
        for (int32 cy = 0; cy < 4; cy++)
        {
            for (int32 cx = 0; cx < 4; cx++)
                Helper.AddVertex(FVector2D(gridX[cx] / TextureScale.X, gridY[cy] / TextureScale.Y), Helper.VertexColor, FVector2D(gridTexX[cx], gridTexY[cy]));
        }
        Helper.AddTriangles(TRIANGLES_9_GRID, sizeof(TRIANGLES_9_GRID) / sizeof(SlateIndex));
    }
    else
    {
        FBox2D drawRect;
        FBox2D texRect;
        int32 row, col;
        int32 part;

        for (int32 pii = 0; pii < 9; pii++)
        {
            col = pii % 3;
            row = pii / 3;
            part = gridTileIndice[pii];
            drawRect = FBox2D(FVector2D(gridX[col], gridY[row]), FVector2D(gridX[col + 1], gridY[row + 1]));
            texRect = FBox2D(FVector2D(gridTexX[col], gridTexY[row]), FVector2D(gridTexX[col + 1], gridTexY[row + 1]));

            if (part != -1 && (TileGridIndice & (1 << part)) != 0)
            {
                TileFill(Helper, drawRect, texRect,
                    (part == 0 || part == 1 || part == 4) ? GridRect.GetSize() : drawRect.GetSize());
            }
            else
            {
                drawRect.Min /= TextureScale;
                drawRect.Max = drawRect.Min + drawRect.GetSize() * TextureScale;

                Helper.AddQuad(drawRect, Helper.VertexColor, texRect);
            }
        }

        Helper.AddTriangles();
    }
}

void SFImage::TileFill(FVertexHelper& Helper, const FBox2D& ContentRect, const FBox2D& UVRect, const FVector2D& TextureSize)
{
    FVector2D cnt = ContentRect.GetSize() / TextureSize;
    cnt.Set(FMath::CeilToInt(cnt.X), FMath::CeilToInt(cnt.Y));
    FVector2D tailSize = ContentRect.GetSize() - (cnt - FVector2D(1, 1)) * TextureSize;

    for (int32 i = 0; i < cnt.X; i++)
    {
        for (int32 j = 0; j < cnt.Y; j++)
        {
            FBox2D UVTmp = UVRect;
            if (i == cnt.X - 1)
                UVTmp.Max.X = FMath::Lerp(UVRect.Min.X, UVRect.Max.X, tailSize.X / TextureSize.X);
            if (j == cnt.Y - 1)
                UVTmp.Max.Y = FMath::Lerp(UVRect.Min.Y, UVRect.Max.Y, tailSize.Y / TextureSize.Y);

            FVector2D Min = ContentRect.Min + FVector2D(i, j) * TextureSize;
            FBox2D drawRect = FBox2D(Min,
                Min + FVector2D(i == (cnt.X - 1) ? tailSize.X : TextureSize.X, j == (cnt.Y - 1) ? tailSize.Y : TextureSize.Y));

            drawRect.Min /= TextureScale;
            drawRect.Max = drawRect.Min + drawRect.GetSize() / TextureScale;

            Helper.AddQuad(drawRect, Helper.VertexColor, UVTmp);
        }
    }
}
