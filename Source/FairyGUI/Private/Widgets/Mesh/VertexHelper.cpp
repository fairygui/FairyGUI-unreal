#include "Widgets/Mesh/VertexHelper.h"

FVertexHelper::FVertexHelper() :
    ContentRect(ForceInit),
    UVRect(ForceInit),
    VertexColor(FColor::White)
{
}

void FVertexHelper::Clear()
{
    Vertices.Reset();
}

int32 FVertexHelper::GetVertexCount() const
{
    return Vertices.Num();
}

void FVertexHelper::AddVertex(const FVector2D& Position)
{
    AddVertex(Position, VertexColor);
}

void FVertexHelper::AddVertex(const FVector2D& Position, const FColor& Color)
{
    AddVertex(Position, Color, FMath::Lerp(UVRect.Min, UVRect.Max, (Position - ContentRect.Min) / ContentRect.GetSize()));
}

void FVertexHelper::AddVertex(const FVector2D& Position, const FColor& Color, const FVector2D& TexCoords)
{
    FSlateVertex Vertex;
    Vertex.Position = Position;
    Vertex.Color = Color;
    Vertex.TexCoords[0] = TexCoords.X;
    Vertex.TexCoords[1] = TexCoords.Y;
    Vertex.TexCoords[2] = 1;
    Vertex.TexCoords[3] = 1;
    Vertex.MaterialTexCoords[0] = TexCoords.X;
    Vertex.MaterialTexCoords[1] = TexCoords.Y;
    Vertices.Add(Vertex);
}

void FVertexHelper::AddQuad(const FBox2D& VertRect)
{
    AddQuad(VertRect, VertexColor);
}

void FVertexHelper::AddQuad(const FBox2D& VertRect, const FColor& Color)
{
    AddVertex(FVector2D(VertRect.Min.X, VertRect.Max.Y), Color);
    AddVertex(FVector2D(VertRect.Min.X, VertRect.Min.Y), Color);
    AddVertex(FVector2D(VertRect.Max.X, VertRect.Min.Y), Color);
    AddVertex(FVector2D(VertRect.Max.X, VertRect.Max.Y), Color);
}

void FVertexHelper::AddQuad(const FBox2D& VertRect, const FColor& Color, const FBox2D& InUVRect)
{
    AddVertex(FVector2D(VertRect.Min.X, VertRect.Max.Y), Color, FVector2D(InUVRect.Min.X, InUVRect.Max.Y));
    AddVertex(FVector2D(VertRect.Min.X, VertRect.Min.Y), Color, FVector2D(InUVRect.Min.X, InUVRect.Min.Y));
    AddVertex(FVector2D(VertRect.Max.X, VertRect.Min.Y), Color, FVector2D(InUVRect.Max.X, InUVRect.Min.Y));
    AddVertex(FVector2D(VertRect.Max.X, VertRect.Max.Y), Color, FVector2D(InUVRect.Max.X, InUVRect.Max.Y));
}

void FVertexHelper::RepeatColors(FColor* Colors, int32 ColorCount, int32 StartIndex, int32 Count)
{
    int32 len = FMath::Min(StartIndex + Count, Vertices.Num());
    int32 k = 0;
    for (int32 i = StartIndex; i < len; i++)
    {
        Vertices[i].Color = Colors[(k++) % ColorCount];
    }
}

void FVertexHelper::AddTriangle(SlateIndex idx0, SlateIndex idx1, SlateIndex idx2)
{
    Triangles.Add(idx0);
    Triangles.Add(idx1);
    Triangles.Add(idx2);
}

void FVertexHelper::AddTriangles(const SlateIndex* Indice, int32 IndiceLength, int32 StartVertexIndex)
{
    if (StartVertexIndex != 0)
    {
        if (StartVertexIndex < 0)
            StartVertexIndex = Vertices.Num() + StartVertexIndex;

        for (int32 i = 0; i < IndiceLength; i++)
            Triangles.Add(Indice[i] + StartVertexIndex);
    }
    else
    {
        Triangles.Append(Indice, IndiceLength);
    }
}

void FVertexHelper::AddTriangles(int32 StartVertexIndex)
{
    int32 cnt = Vertices.Num();
    if (StartVertexIndex < 0)
        StartVertexIndex = cnt + StartVertexIndex;

    for (int32 i = StartVertexIndex; i < cnt; i += 4)
    {
        Triangles.Add(i);
        Triangles.Add(i + 1);
        Triangles.Add(i + 2);

        Triangles.Add(i + 2);
        Triangles.Add(i + 3);
        Triangles.Add(i);
    }
}

const FVector2D& FVertexHelper::GetPosition(int32 Index)
{
    if (Index < 0)
        Index = Vertices.Num() + Index;

    return Vertices[Index].Position;
}

FVector2D FVertexHelper::GetUVAtPosition(const FVector2D& Position, bool bUsePercent)
{
    if (bUsePercent)
        return FMath::Lerp(UVRect.Min, UVRect.Max, Position);
    else
        return FMath::Lerp(UVRect.Min, UVRect.Max, (Position - ContentRect.Min) / ContentRect.GetSize());
}

void FVertexHelper::Append(const FVertexHelper& VertexHelper)
{
    Vertices += VertexHelper.Vertices;
    Triangles += VertexHelper.Triangles;
}

void FVertexHelper::Insert(const FVertexHelper& VertexHelper)
{
    Vertices.Insert(VertexHelper.Vertices.GetData(), VertexHelper.Vertices.Num(), 0);
    Triangles.Insert(VertexHelper.Triangles.GetData(), VertexHelper.Triangles.Num(), 0);
}