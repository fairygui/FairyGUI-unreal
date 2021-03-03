#pragma once

#include "MeshFactory.h"

class FAIRYGUI_API FPolygonMesh : public IMeshFactory
{
public:
    MESHFACTORY_TYPE(FPolygonMesh)

    FPolygonMesh();
    virtual ~FPolygonMesh() {}

    TArray<FVector2D> Points;
    TArray<FVector2D> Texcoords;
    float LineWidth;
    FColor LineColor;
    TOptional<FColor> FillColor;
    TOptional<TArray<FColor>> Colors;
    bool bUsePercentPositions;

    void OnPopulateMesh(FVertexHelper& Helper);

private:
    void DrawOutline(FVertexHelper& Helper);
    bool IsPointInTriangle(const FVector2D& p, const FVector2D& a, const FVector2D& b, const FVector2D& c);
};