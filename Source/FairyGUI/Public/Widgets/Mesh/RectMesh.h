#pragma once

#include "MeshFactory.h"

class FAIRYGUI_API FRectMesh : public IMeshFactory
{
public:
    MESHFACTORY_TYPE(FRectMesh, nullptr)

    FRectMesh();
    virtual ~FRectMesh() {}

    TOptional<FBox2D> DrawRect;
    float LineWidth;
    FColor LineColor;
    TOptional<FColor> FillColor;
    TOptional<TArray<FColor>> Colors;

    void OnPopulateMesh(FVertexHelper& Helper);
};