#pragma once

#include "MeshFactory.h"
#include "Widgets/HitTest.h"

class FAIRYGUI_API FEllipseMesh : public IMeshFactory, public IHitTest
{
public:
    MESHFACTORY_TYPE(FEllipseMesh, this)

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
    bool HitTest(const FBox2D& ContentRect, const FVector2D& LayoutScaleMultiplier, const FVector2D& LocalPoint) const;
};