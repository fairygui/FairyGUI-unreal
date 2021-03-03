#pragma once

#include "VertexHelper.h"

#define MESHFACTORY_TYPE(TYPE) \
	static const FName& GetMeshFactoryTypeId() { static FName Type(TEXT(#TYPE)); return Type; } \
	virtual bool IsMeshFactoryOfType(const FName& Type) const override { return GetMeshFactoryTypeId() == Type; }

class FAIRYGUI_API IMeshFactory
{
public:
    virtual void OnPopulateMesh(FVertexHelper& Helper) = 0;
    virtual bool IsMeshFactoryOfType(const FName& Type) const = 0;
};

class FAIRYGUI_API FMeshFactory : public  IMeshFactory
{
public:
    FMeshFactory(IMeshFactory* InSourceFactory)
    {
        SourceFactory = InSourceFactory;
    }

    inline virtual void OnPopulateMesh(FVertexHelper& Helper) override
    {
        SourceFactory->OnPopulateMesh(Helper);
    }

    inline virtual bool IsMeshFactoryOfType(const FName& Type) const override
    {
        return SourceFactory->IsMeshFactoryOfType(Type);
    }

    IMeshFactory* SourceFactory;
};