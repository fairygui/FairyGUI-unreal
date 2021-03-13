#pragma once

#include "MeshFactory.h"

class FAIRYGUI_API FRegularPolygonMesh : public IMeshFactory
{
public:
    MESHFACTORY_TYPE(FRegularPolygonMesh, nullptr)

    FRegularPolygonMesh();
    virtual ~FRegularPolygonMesh() {}

    TOptional<FBox2D> DrawRect;
    int32 Sides;
    float LineWidth;
    FColor LineColor;
    TOptional<FColor> CenterColor;
    TOptional<FColor> FillColor;
    TArray<float> Distances;
    float Rotation;

    void OnPopulateMesh(FVertexHelper& Helper);
};