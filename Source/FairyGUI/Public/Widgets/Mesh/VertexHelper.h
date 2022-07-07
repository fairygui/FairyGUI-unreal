#pragma once

#include "Slate.h"

class FAIRYGUI_API FVertexHelper
{
public:
    FVertexHelper();

    void Clear();
    int32 GetVertexCount() const;

    void AddVertex(const FVector2D& Position);
    void AddVertex(const FVector2D& Position, const FColor& Color);
    void AddVertex(const FVector2D& Position, const FColor& Color, const FVector2D& TexCoords);

    void AddQuad(const FBox2D& VertRect);
    void AddQuad(const FBox2D& VertRect, const FColor& Color);
    void AddQuad(const FBox2D& VertRect, const FColor& Color, const FBox2D& InUVRect);

    void RepeatColors(FColor* Colors, int32 ColorCount, int32 StartIndex, int32 Count);

    void AddTriangle(SlateIndex idx0, SlateIndex idx1, SlateIndex idx2);
    void AddTriangles(const SlateIndex* Indice, int32 IndiceLength, int32 StartVertexIndex = 0);
    void AddTriangles(int32 StartVertexIndex = 0);

    const FVector2D GetPosition(int32 Index);
    FVector2D GetUVAtPosition(const FVector2D& Position, bool bUsePercent);

    void Append(const FVertexHelper& VertexHelper);
    void Insert(const FVertexHelper& VertexHelper);

public:
    FBox2D ContentRect;
    FBox2D UVRect;
    FColor VertexColor;
    FVector2D TextureSize;

    TArray<FSlateVertex> Vertices;
    TArray<SlateIndex> Triangles;
};