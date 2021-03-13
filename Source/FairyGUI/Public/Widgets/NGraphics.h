#pragma once

#include "Mesh/MeshFactory.h"
#include "NTexture.h"
#include "UI/FieldTypes.h"

class FAIRYGUI_API FNGraphics : public FGCObject
{
public:
    FNGraphics();
    virtual ~FNGraphics();

    const FColor& GetColor() const { return Color; }
    void SetColor(const FColor& InColor);

    EFlipType GetFlip() const { return Flip; }
    void SetFlip(EFlipType Value);

    void SetTexture(UNTexture* InTexture);
    UNTexture* GetTexture() const { return Texture; }

    void SetMeshFactory(const TSharedPtr<IMeshFactory>& InMeshFactory);
    const TSharedPtr<IMeshFactory>& GetMeshFactory() { return MeshFactory; }
    template <typename T> T& GetMeshFactory();

    void SetMeshDirty() { bMeshDirty = true; }

    void Paint(const FGeometry& AllottedGeometry,
        FSlateWindowElementList& OutDrawElements,
        int32 LayerId,
        float Alpha,
        bool bEnabled);

    void PopulateDefaultMesh(FVertexHelper& Helper);

    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

private:
    void UpdateMeshNow();

    FVector2D Size;
    FColor Color;
    EFlipType Flip;

    UNTexture* Texture;
    FSlateBrush Brush;
    FSlateResourceHandle ResourceHandle;
    TSharedPtr<IMeshFactory> MeshFactory;

    TArray<FSlateVertex> Vertices;
    TArray<SlateIndex> Triangles;
    TArray<FVector2D> PositionsBackup;
    TArray<float> AlphaBackup;
    float UsingAlpha;
    bool bMeshDirty;
};

template <typename T>
inline T& FNGraphics::GetMeshFactory()
{
    if (MeshFactory.IsValid() && MeshFactory->IsMeshFactoryOfType(T::GetMeshFactoryTypeId()))
        return *static_cast<T*>(MeshFactory.Get());

    T* Ret = new T();
    MeshFactory = MakeShareable(Ret);
    return *Ret;
}