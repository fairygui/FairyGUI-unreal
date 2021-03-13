#pragma once

#include "MeshFactory.h"

class FAIRYGUI_API FPolygonMesh : public IMeshFactory, public IHitTest
{
public:
    MESHFACTORY_TYPE(FPolygonMesh, this)

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
    bool HitTest(const FBox2D& ContentRect, const FVector2D& LayoutScaleMultiplier, const FVector2D& LocalPoint) const;

private:
    void DrawOutline(FVertexHelper& Helper);
    bool IsPointInTriangle(const FVector2D& p, const FVector2D& a, const FVector2D& b, const FVector2D& c);
};