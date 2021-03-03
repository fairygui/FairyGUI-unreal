#include "Widgets/Mesh/RoundedRectMesh.h"

FRoundedRectMesh::FRoundedRectMesh() :
    LineWidth(0),
    LineColor(FColor::Black),
    TopLeftRadius(0),
    TopRightRadius(0),
    BottomLeftRadius(0),
    BottomRightRadius(0)
{
}

void FRoundedRectMesh::OnPopulateMesh(FVertexHelper& Helper)
{
    const FBox2D& rect = DrawRect.IsSet() ? DrawRect.GetValue() : Helper.ContentRect;
    const FColor& color = FillColor.IsSet() ? FillColor.GetValue() : Helper.VertexColor;

    FVector2D radius = rect.GetSize() * 0.5f;
    float cornerMaxRadius = radius.GetMin();
    FVector2D center = radius + rect.Min;

    Helper.AddVertex(center, color);

    int32 cnt = Helper.GetVertexCount();
    for (int32 i = 0; i < 4; i++)
    {
        float cornerRadius = 0;
        switch (i)
        {
        case 0:
            cornerRadius = BottomRightRadius;
            break;

        case 1:
            cornerRadius = BottomLeftRadius;
            break;

        case 2:
            cornerRadius = TopLeftRadius;
            break;

        case 3:
            cornerRadius = TopRightRadius;
            break;
        }
        cornerRadius = FMath::Min(cornerMaxRadius, cornerRadius);

        FVector2D offset = rect.Min;

        if (i == 0 || i == 3)
            offset.X = rect.Max.X - cornerRadius * 2;
        if (i == 0 || i == 1)
            offset.Y = rect.Max.Y - cornerRadius * 2;

        if (cornerRadius != 0)
        {
            int32 partNumSides = FMath::Max(1, FMath::CeilToInt(PI * cornerRadius / 8)) + 1;
            float angleDelta = PI / 2 / partNumSides;
            float angle = PI / 2 * i;
            float startAngle = angle;

            for (int32 j = 1; j <= partNumSides; j++)
            {
                if (j == partNumSides)
                    angle = startAngle + PI / 2;
                FVector2D v1(offset.X + FMath::Cos(angle) * (cornerRadius - LineWidth) + cornerRadius,
                    offset.Y + FMath::Sin(angle) * (cornerRadius - LineWidth) + cornerRadius);
                Helper.AddVertex(v1, color);
                if (LineWidth != 0)
                {
                    Helper.AddVertex(v1, LineColor);
                    Helper.AddVertex(FVector2D(offset.X + FMath::Cos(angle) * cornerRadius + cornerRadius,
                        offset.Y + FMath::Sin(angle) * cornerRadius + cornerRadius), LineColor);
                }
                angle += angleDelta;
            }
        }
        else
        {
            FVector2D v1 = offset;
            if (LineWidth != 0)
            {
                if (i == 0 || i == 3)
                    offset.X -= LineWidth;
                else
                    offset.X += LineWidth;
                if (i == 0 || i == 1)
                    offset.Y -= LineWidth;
                else
                    offset.Y += LineWidth;
                Helper.AddVertex(offset, color);
                Helper.AddVertex(offset, LineColor);
                Helper.AddVertex(v1, LineColor);
            }
            else
                Helper.AddVertex(v1, color);
        }
    }
    cnt = Helper.GetVertexCount() - cnt;

    if (LineWidth > 0)
    {
        for (int32 i = 0; i < cnt; i += 3)
        {
            if (i != cnt - 3)
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
        for (int32 i = 0; i < cnt; i++)
            Helper.AddTriangle(0, i + 1, (i == cnt - 1) ? 1 : i + 2);
    }
}
