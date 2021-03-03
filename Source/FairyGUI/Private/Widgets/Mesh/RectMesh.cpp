#include "Widgets/Mesh/RectMesh.h"

FRectMesh::FRectMesh() :
    LineWidth(0),
    LineColor(FColor::Black)
{
}

void FRectMesh::OnPopulateMesh(FVertexHelper& Helper)
{
    const FBox2D& rect = DrawRect.IsSet() ? DrawRect.GetValue() : Helper.ContentRect;
    const FColor& color = FillColor.IsSet() ? FillColor.GetValue() : Helper.VertexColor;
    if (LineWidth == 0)
    {
        if (color.A != 0)//optimized
            Helper.AddQuad(rect, color);
    }
    else
    {
        FBox2D part;
        FVector2D Position;

        //left,right
        part = FBox2D(rect.Min, rect.Min + FVector2D(LineWidth, rect.GetSize().Y));
        Helper.AddQuad(part, LineColor);
        Position = FVector2D(rect.Max.X - LineWidth, rect.Min.Y);
        part = FBox2D(Position, Position + FVector2D(LineWidth, rect.GetSize().Y));
        Helper.AddQuad(part, LineColor);

        //top, bottom
        Position = FVector2D(rect.Min.X + LineWidth, rect.Min.Y);
        part = FBox2D(Position, Position + FVector2D(rect.GetSize().X - LineWidth * 2, LineWidth));
        Helper.AddQuad(part, LineColor);
        Position = FVector2D(rect.Min.X + LineWidth, rect.Max.Y - LineWidth);
        part = FBox2D(Position, Position + FVector2D(rect.GetSize().X - LineWidth * 2, LineWidth));
        Helper.AddQuad(part, LineColor);

        //middle
        if (color.A != 0)//optimized
        {
            part = FBox2D(rect.Min + LineWidth, rect.Max - LineWidth);
            if (part.GetSize().GetMin() > 0)
                Helper.AddQuad(part, color);
        }
    }

    if (Colors.IsSet())
        Helper.RepeatColors(Colors.GetValue().GetData(), Colors.GetValue().Num(), 0, Helper.GetVertexCount());

    Helper.AddTriangles();
}
