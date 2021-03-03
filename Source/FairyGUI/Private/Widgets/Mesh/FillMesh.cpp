#include "Widgets/Mesh/FillMesh.h"

static void FillHorizontal(FVertexHelper& Helper, FBox2D VertRect, int32 Origin, float Amount)
{
    float a = VertRect.GetSize().X * Amount;
    if ((EOriginHorizontal)Origin == EOriginHorizontal::Right || (EOriginVertical)Origin == EOriginVertical::Bottom)
        VertRect.Min.X += (VertRect.GetSize().X - a);
    else
        VertRect.Max.X = VertRect.Min.X + a;

    Helper.AddQuad(VertRect);
    Helper.AddTriangles();
}

static void FillVertical(FVertexHelper& Helper, FBox2D VertRect, int32 Origin, float Amount)
{
    float a = VertRect.GetSize().Y * Amount;
    if ((EOriginHorizontal)Origin == EOriginHorizontal::Right || (EOriginVertical)Origin == EOriginVertical::Bottom)
        VertRect.Min.Y += (VertRect.GetSize().Y - a);
    else
        VertRect.Max.Y = VertRect.Min.Y + a;

    Helper.AddQuad(VertRect);
    Helper.AddTriangles();
}

//4 vertex
static void FillRadial90(FVertexHelper& Helper, FBox2D VertRect, EOrigin90 Origin, float Amount, bool bClockwise)
{
    bool flipX = Origin == EOrigin90::TopRight || Origin == EOrigin90::BottomRight;
    bool flipY = Origin == EOrigin90::BottomLeft || Origin == EOrigin90::BottomRight;
    if (flipX != flipY)
        bClockwise = !bClockwise;

    float ratio = bClockwise ? Amount : (1 - Amount);
    float tan = FMath::Tan(PI * 0.5f * ratio);
    bool thresold = false;
    if (ratio != 1)
        thresold = (VertRect.GetSize().Y / VertRect.GetSize().X - tan) > 0;
    if (!bClockwise)
        thresold = !thresold;
    float x = VertRect.Min.X + (ratio == 0 ? FLT_MAX : (VertRect.GetSize().Y / tan));
    float y = VertRect.Min.Y + (ratio == 1 ? FLT_MAX : (VertRect.GetSize().X * tan));
    float x2 = x;
    float y2 = y;
    if (flipX)
        x2 = VertRect.GetSize().X - x;
    if (flipY)
        y2 = VertRect.GetSize().Y - y;
    float xMin = flipX ? (VertRect.GetSize().X - VertRect.Min.X) : VertRect.Min.X;
    float yMin = flipY ? (VertRect.GetSize().Y - VertRect.Min.Y) : VertRect.Min.Y;
    float xMax = flipX ? -VertRect.Min.X : VertRect.Max.X;
    float yMax = flipY ? -VertRect.Min.Y : VertRect.Max.Y;

    Helper.AddVertex(FVector2D(xMin, yMin));

    if (bClockwise)
        Helper.AddVertex(FVector2D(xMax, yMin));

    if (y > VertRect.Max.Y)
    {
        if (thresold)
            Helper.AddVertex(FVector2D(x2, yMax));
        else
            Helper.AddVertex(FVector2D(xMax, yMax));
    }
    else
        Helper.AddVertex(FVector2D(xMax, y2));

    if (x > VertRect.Max.X)
    {
        if (thresold)
            Helper.AddVertex(FVector2D(xMax, y2));
        else
            Helper.AddVertex(FVector2D(xMax, yMax));
    }
    else
        Helper.AddVertex(FVector2D(x2, yMax));

    if (!bClockwise)
        Helper.AddVertex(FVector2D(xMin, yMax));

    if (flipX == flipY)
    {
        Helper.AddTriangle(0, 1, 2);
        Helper.AddTriangle(0, 2, 3);
    }
    else
    {
        Helper.AddTriangle(2, 1, 0);
        Helper.AddTriangle(3, 2, 0);
    }
}

#define LEFT_BOX(Rect) FBox2D(Rect.Min, Rect.Max - FVector2D(Rect.GetSize().X * 0.5f, 0))
#define RIGHT_BOX(Rect) FBox2D(Rect.Min + FVector2D(Rect.GetSize().X * 0.5f, 0), Rect.Max)
#define TOP_BOX(Rect) FBox2D(Rect.Min, Rect.Max - FVector2D(0, Rect.GetSize().Y * 0.5f))
#define BOTTOM_BOX(Rect) FBox2D(Rect.Min + FVector2D(0, Rect.GetSize().Y * 0.5f), Rect.Max)

#define LEFT_BOX_CW bClockwise?LEFT_BOX(VertRect):RIGHT_BOX(VertRect)
#define RIGHT_BOX_CW bClockwise?RIGHT_BOX(VertRect):LEFT_BOX(VertRect)
#define TOP_BOX_CW bClockwise?TOP_BOX(VertRect):BOTTOM_BOX(VertRect)
#define BOTTOM_BOX_CW bClockwise?BOTTOM_BOX(VertRect):TOP_BOX(VertRect)

//8 vertex
static void FillRadial180(FVertexHelper& Helper, FBox2D VertRect, EOrigin180 Origin, float Amount, bool bClockwise)
{
    switch (Origin)
    {
    case EOrigin180::Top:
        if (Amount <= 0.5f)
        {
            FillRadial90(Helper, RIGHT_BOX_CW, bClockwise ? EOrigin90::TopLeft : EOrigin90::TopRight, Amount / 0.5f, bClockwise);
        }
        else
        {
            FillRadial90(Helper, LEFT_BOX_CW, bClockwise ? EOrigin90::TopRight : EOrigin90::TopLeft, (Amount - 0.5f) / 0.5f, bClockwise);

            Helper.AddQuad(RIGHT_BOX_CW);
            Helper.AddTriangles(-4);
        }
        break;

    case EOrigin180::Bottom:
        if (Amount <= 0.5f)
        {
            FillRadial90(Helper, LEFT_BOX_CW, bClockwise ? EOrigin90::BottomRight : EOrigin90::BottomLeft, Amount / 0.5f, bClockwise);
        }
        else
        {
            FillRadial90(Helper, RIGHT_BOX_CW, bClockwise ? EOrigin90::BottomLeft : EOrigin90::BottomRight, (Amount - 0.5f) / 0.5f, bClockwise);

            Helper.AddQuad(LEFT_BOX_CW);
            Helper.AddTriangles(-4);
        }
        break;

    case EOrigin180::Left:
        if (Amount <= 0.5f)
        {
            FillRadial90(Helper, TOP_BOX_CW, bClockwise ? EOrigin90::BottomLeft : EOrigin90::TopLeft, Amount / 0.5f, bClockwise);
        }
        else
        {
            FillRadial90(Helper, BOTTOM_BOX_CW, bClockwise ? EOrigin90::TopLeft : EOrigin90::BottomLeft, (Amount - 0.5f) / 0.5f, bClockwise);

            Helper.AddQuad(TOP_BOX_CW);
            Helper.AddTriangles(-4);
        }
        break;

    case EOrigin180::Right:
        if (Amount <= 0.5f)
        {
            FillRadial90(Helper, BOTTOM_BOX_CW, bClockwise ? EOrigin90::TopRight : EOrigin90::BottomRight, Amount / 0.5f, bClockwise);
        }
        else
        {
            FillRadial90(Helper, TOP_BOX_CW, bClockwise ? EOrigin90::BottomRight : EOrigin90::TopRight, (Amount - 0.5f) / 0.5f, bClockwise);

            Helper.AddQuad(BOTTOM_BOX_CW);
            Helper.AddTriangles(-4);
        }
        break;
    }
}

//12 vertex
static void FillRadial360(FVertexHelper& Helper, FBox2D VertRect, EOrigin360 Origin, float Amount, bool bClockwise)
{
    switch (Origin)
    {
    case EOrigin360::Top:
        if (Amount < 0.5f)
        {
            FillRadial180(Helper, RIGHT_BOX_CW, bClockwise ? EOrigin180::Left : EOrigin180::Right, Amount / 0.5f, bClockwise);
        }
        else
        {
            FillRadial180(Helper, LEFT_BOX_CW, bClockwise ? EOrigin180::Right : EOrigin180::Left, (Amount - 0.5f) / 0.5f, bClockwise);

            Helper.AddQuad(RIGHT_BOX_CW);
            Helper.AddTriangles(-4);
        }

        break;

    case EOrigin360::Bottom:
        if (Amount < 0.5f)
        {
            FillRadial180(Helper, LEFT_BOX_CW, bClockwise ? EOrigin180::Right : EOrigin180::Left, Amount / 0.5f, bClockwise);
        }
        else
        {
            FillRadial180(Helper, RIGHT_BOX_CW, bClockwise ? EOrigin180::Left : EOrigin180::Right, (Amount - 0.5f) / 0.5f, bClockwise);

            Helper.AddQuad(LEFT_BOX_CW);
            Helper.AddTriangles(-4);
        }
        break;

    case EOrigin360::Left:
        if (Amount < 0.5f)
        {
            FillRadial180(Helper, TOP_BOX_CW, bClockwise ? EOrigin180::Bottom : EOrigin180::Top, Amount / 0.5f, bClockwise);
        }
        else
        {
            FillRadial180(Helper, BOTTOM_BOX_CW, bClockwise ? EOrigin180::Top : EOrigin180::Bottom, (Amount - 0.5f) / 0.5f, bClockwise);

            Helper.AddQuad(TOP_BOX_CW);
            Helper.AddTriangles(-4);
        }
        break;

    case EOrigin360::Right:
        if (Amount < 0.5f)
        {
            FillRadial180(Helper, BOTTOM_BOX_CW, bClockwise ? EOrigin180::Top : EOrigin180::Bottom, Amount / 0.5f, bClockwise);
        }
        else
        {
            FillRadial180(Helper, TOP_BOX_CW, bClockwise ? EOrigin180::Bottom : EOrigin180::Top, (Amount - 0.5f) / 0.5f, bClockwise);

            Helper.AddQuad(BOTTOM_BOX_CW);
            Helper.AddTriangles(-4);
        }
        break;
    }
}

FFillMesh::FFillMesh() :
    Method(EFillMethod::None),
    Origin(0),
    bClockwise(true),
    Amount(1)
{
}

void FFillMesh::OnPopulateMesh(FVertexHelper& Helper)
{
    const float clampedAmount = FMath::Clamp<float>(Amount, 0, 1);
    switch (Method)
    {
    case EFillMethod::Horizontal:
        FillHorizontal(Helper, Helper.ContentRect, Origin, clampedAmount);
        break;

    case EFillMethod::Vertical:
        FillVertical(Helper, Helper.ContentRect, Origin, clampedAmount);
        break;

    case EFillMethod::Radial90:
        FillRadial90(Helper, Helper.ContentRect, (EOrigin90)Origin, clampedAmount, bClockwise);
        break;

    case EFillMethod::Radial180:
        FillRadial180(Helper, Helper.ContentRect, (EOrigin180)Origin, clampedAmount, bClockwise);
        break;

    case EFillMethod::Radial360:
        FillRadial360(Helper, Helper.ContentRect, (EOrigin360)Origin, clampedAmount, bClockwise);
        break;
    }
}
