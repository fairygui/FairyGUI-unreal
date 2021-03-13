#include "Widgets/Mesh/EllipseMesh.h"

FEllipseMesh::FEllipseMesh() :
    LineWidth(0),
    LineColor(FColor::Black),
    StartDegree(0),
    EndDegreee(360)
{
}

void FEllipseMesh::OnPopulateMesh(FVertexHelper& Helper)
{
    static SlateIndex SECTOR_CENTER_TRIANGLES[] = {
        0, 4, 1,
        0, 3, 4,
        0, 2, 3,
        0, 8, 5,
        0, 7, 8,
        0, 6, 7,
        6, 5, 2,
        2, 1, 6
    };

    const FBox2D& rect = DrawRect.IsSet() ? DrawRect.GetValue() : Helper.ContentRect;
    const FColor& color = FillColor.IsSet() ? FillColor.GetValue() : Helper.VertexColor;

    float sectionStart = FMath::Clamp<float>(StartDegree, 0, 360);
    float sectionEnd = FMath::Clamp<float>(EndDegreee, 0, 360);
    bool clipped = sectionStart > 0 || sectionEnd < 360;
    sectionStart = FMath::DegreesToRadians(sectionStart);
    sectionEnd = FMath::DegreesToRadians(sectionEnd);
    const FColor& centerColor2 = CenterColor.IsSet() ? CenterColor.GetValue() : color;

    FVector2D radius = rect.GetSize() * 0.5f;
    int32 sides = FMath::CeilToInt(PI * (radius.X + radius.Y) / 4);
    sides = FMath::Clamp<int32>(sides, 40, 800);
    float angleDelta = 2 * PI / sides;
    float angle = 0;
    float lineAngle = 0;

    if (LineWidth > 0 && clipped)
    {
        lineAngle = LineWidth / radius.GetMax();
        sectionStart += lineAngle;
        sectionEnd -= lineAngle;
    }

    int32 vpos = Helper.GetVertexCount();
    FVector2D center = rect.Min + radius;
    Helper.AddVertex(center, centerColor2);
    for (int32 i = 0; i < sides; i++)
    {
        if (angle < sectionStart)
            angle = sectionStart;
        else if (angle > sectionEnd)
            angle = sectionEnd;
        FVector2D vec(FMath::Cos(angle) * (radius.X - LineWidth) + center.X, FMath::Sin(angle) * (radius.Y - LineWidth) + center.Y);
        Helper.AddVertex(vec, color);
        if (LineWidth > 0)
        {
            Helper.AddVertex(vec, LineColor);
            Helper.AddVertex(FVector2D(FMath::Cos(angle) * radius.X + center.X, FMath::Sin(angle) * radius.Y + center.Y), LineColor);
        }
        angle += angleDelta;
    }

    if (LineWidth > 0)
    {
        int32 cnt = sides * 3;
        for (int32 i = 0; i < cnt; i += 3)
        {
            if (i != cnt - 3)
            {
                Helper.AddTriangle(0, i + 1, i + 4);
                Helper.AddTriangle(i + 5, i + 2, i + 3);
                Helper.AddTriangle(i + 3, i + 6, i + 5);
            }
            else if (!clipped)
            {
                Helper.AddTriangle(0, i + 1, 1);
                Helper.AddTriangle(2, i + 2, i + 3);
                Helper.AddTriangle(i + 3, 3, 2);
            }
            else
            {
                Helper.AddTriangle(0, i + 1, i + 1);
                Helper.AddTriangle(i + 2, i + 2, i + 3);
                Helper.AddTriangle(i + 3, i + 3, i + 2);
            }
        }
    }
    else
    {
        for (int32 i = 0; i < sides; i++)
        {
            if (i != sides - 1)
                Helper.AddTriangle(0, i + 1, i + 2);
            else if (!clipped)
                Helper.AddTriangle(0, i + 1, 1);
            else
                Helper.AddTriangle(0, i + 1, i + 1);
        }
    }

    if (LineWidth > 0 && clipped)
    {
        Helper.AddVertex(FVector2D(radius.X, radius.Y), LineColor);
        float centerRadius = LineWidth * 0.5f;

        sectionStart -= lineAngle;
        angle = sectionStart + lineAngle * 0.5f + PI * 0.5f;
        Helper.AddVertex(FVector2D(FMath::Cos(angle) * centerRadius + radius.X, FMath::Sin(angle) * centerRadius + radius.Y), LineColor);
        angle -= PI;
        Helper.AddVertex(FVector2D(FMath::Cos(angle) * centerRadius + radius.X, FMath::Sin(angle) * centerRadius + radius.Y), LineColor);
        Helper.AddVertex(FVector2D(FMath::Cos(sectionStart) * radius.X + radius.X, FMath::Sin(sectionStart) * radius.Y + radius.Y), LineColor);
        Helper.AddVertex(Helper.GetPosition(vpos + 3), LineColor);

        sectionEnd += lineAngle;
        angle = sectionEnd - lineAngle * 0.5f + PI * 0.5f;
        Helper.AddVertex(FVector2D(FMath::Cos(angle) * centerRadius + radius.X, FMath::Sin(angle) * centerRadius + radius.Y), LineColor);
        angle -= PI;
        Helper.AddVertex(FVector2D(FMath::Cos(angle) * centerRadius + radius.X, FMath::Sin(angle) * centerRadius + radius.Y), LineColor);
        Helper.AddVertex(Helper.GetPosition(vpos + sides * 3), LineColor);
        Helper.AddVertex(FVector2D(FMath::Cos(sectionEnd) * radius.X + radius.X, FMath::Sin(sectionEnd) * radius.Y + radius.Y), LineColor);

        Helper.AddTriangles(SECTOR_CENTER_TRIANGLES, 24, sides * 3 + 1);
    }
}

bool FEllipseMesh::HitTest(const FBox2D& ContentRect, const FVector2D& LayoutScaleMultiplier, const FVector2D& LocalPoint) const
{
    FVector2D Radius = ContentRect.GetSize() * 0.5f;
    FVector2D Pos = LocalPoint - Radius - ContentRect.Min;
    if (FMath::Pow(Pos.X / Radius.X, 2) + FMath::Pow(Pos.Y / Radius.Y, 2) < 1)
    {
        if (StartDegree != 0 || EndDegreee != 360)
        {
            float deg = FMath::RadiansToDegrees(FMath::Atan2(Pos.Y, Pos.X));
            if (deg < 0)
                deg += 360;
            return deg >= StartDegree && deg <= EndDegreee;
        }
        else
            return true;
    }

    return false;
}