#pragma once

#include "MeshFactory.h"
#include "UI/FieldTypes.h"

class FAIRYGUI_API FFillMesh : public IMeshFactory
{
public:
    MESHFACTORY_TYPE(FFillMesh, nullptr)

    FFillMesh();
    virtual ~FFillMesh() {}

    EFillMethod Method;
    int32 Origin;
    bool bClockwise;
    float Amount;

    void OnPopulateMesh(FVertexHelper& Helper);
};