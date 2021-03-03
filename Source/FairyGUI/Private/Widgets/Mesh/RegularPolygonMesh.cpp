#include "Widgets/Mesh/RegularPolygonMesh.h"

FRegularPolygonMesh::FRegularPolygonMesh() :
    Sides(3),
    LineWidth(0),
    LineColor(FColor::Black),
    Rotation(0)
{
}

void FRegularPolygonMesh::OnPopulateMesh(FVertexHelper& Helper)
{
    if (Distances.Num() > 0)
        verifyf(Distances.Num() >= Sides, TEXT("Distances.Length<Sides"));

    const FBox2D& rect = DrawRect.IsSet() ? DrawRect.GetValue() : Helper.ContentRect;
    const FColor& color = FillColor.IsSet() ? FillColor.GetValue() : Helper.VertexColor;

    float angleDelta = 2 * PI / Sides;
    float angle = FMath::DegreesToRadians(Rotation);
    float radius = rect.GetSize().GetMin() *0.5f;

    FVector2D center = FVector2D(radius, radius) + rect.Min;
    Helper.AddVertex(center, CenterColor.IsSet() ? CenterColor.GetValue() : color);
    for (int32 i = 0; i < Sides; i++)
    {
        float r = radius;
        if (Distances.Num() > 0)
            r *= Distances[i];
        FVector2D vec = center + FVector2D(FMath::Cos(angle) * (r - LineWidth),
            FMath::Sin(angle) * (r - LineWidth));
        Helper.AddVertex(vec, color);
        if (LineWidth > 0)
        {
            Helper.AddVertex(vec, LineColor);

            vec = FVector2D(FMath::Cos(angle) * r + center.X,
                FMath::Sin(angle) * r + center.Y);
            Helper.AddVertex(vec, LineColor);
        }
        angle += angleDelta;
    }

    if (LineWidth > 0)
    {
        int32 tmp = Sides * 3;
        for (int32 i = 0; i < tmp; i += 3)
        {
            if (i != tmp - 3)
            {
                Helper.AddTriangle(0, i + 1, i + 4);
                Helper.AddTriangle(i + 5, i + 2, i + 3);
                Helper.AddTriangle(i + 3, i + 6, i + 5);
            }
            else
            {
                Helper.AddTriangle(0, i + 1, 1);
                Helper.AddTriangle(2, i + 2, i + 3);
                Helper.AddTriangle(i + 3, 3, 2);
            }
        }
    }
    else
    {
        for (int32 i = 0; i < Sides; i++)
            Helper.AddTriangle(0, i + 1, (i == Sides - 1) ? 1 : i + 2);
    }
}
