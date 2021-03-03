#pragma once

#include "MeshFactory.h"

class FAIRYGUI_API FEllipseMesh : public IMeshFactory
{
public:
    MESHFACTORY_TYPE(FEllipseMesh)

    FEllipseMesh();
    virtual ~FEllipseMesh() {}

    TOptional<FBox2D> DrawRect;
    float LineWidth;
    FColor LineColor;
    TOptional<FColor> CenterColor;
    TOptional<FColor> FillColor;
    float StartDegree;
    float EndDegreee;

    void OnPopulateMesh(FVertexHelper& Helper);
};