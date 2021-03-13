#include "UI/GGraph.h"
#include "Utils/ByteBuffer.h"
#include "Widgets/NTexture.h"
#include "Widgets/SShape.h"
#include "Widgets/Mesh/RectMesh.h"
#include "Widgets/Mesh/RoundedRectMesh.h"
#include "Widgets/Mesh/PolygonMesh.h"
#include "Widgets/Mesh/RegularPolygonMesh.h"
#include "Widgets/Mesh/EllipseMesh.h"

UGGraph::UGGraph()
{
    if (!HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
        DisplayObject = Content = SNew(SShape).GObject(this);
}

UGGraph::~UGGraph()
{

}

FColor UGGraph::GetColor() const
{
    return Content->Graphics.GetColor();
}

void UGGraph::SetColor(const FColor& InColor)
{
    Content->Graphics.SetColor(InColor);
}

void UGGraph::DrawRect(float LineWidth, const FColor& LineColor, const FColor& FillColor)
{
    FRectMesh& mesh = Content->Graphics.GetMeshFactory<FRectMesh>();
    mesh.LineWidth = LineWidth;
    mesh.LineColor = LineColor;
    mesh.FillColor.Reset();
    mesh.Colors.Reset();

    Content->Graphics.SetColor(FillColor);
    Content->Graphics.SetMeshDirty();
}

void UGGraph::DrawRoundRect(float LineWidth, const FColor& lineColor, const FColor& FillColor,
    float TopLeftRadius, float TopRightRadius, float BottomLeftRadius, float BottomRightRadius)
{
    FRoundedRectMesh& mesh = Content->Graphics.GetMeshFactory<FRoundedRectMesh>();
    mesh.LineWidth = LineWidth;
    mesh.LineColor = lineColor;
    mesh.FillColor.Reset();
    mesh.TopLeftRadius = TopLeftRadius;
    mesh.TopRightRadius = TopRightRadius;
    mesh.BottomLeftRadius = BottomLeftRadius;
    mesh.BottomRightRadius = BottomRightRadius;

    Content->Graphics.SetColor(FillColor);
    Content->Graphics.SetMeshDirty();
}

void UGGraph::DrawEllipse(float LineWidth, const FColor& LineColor, const FColor& FillColor, float StartDegree, float EndDegree)
{
    FEllipseMesh& mesh = Content->Graphics.GetMeshFactory<FEllipseMesh>();
    mesh.LineWidth = LineWidth;
    mesh.LineColor = LineColor;
    mesh.FillColor.Reset();
    mesh.CenterColor.Reset();
    mesh.StartDegree = StartDegree;
    mesh.EndDegreee = EndDegree;

    Content->Graphics.SetColor(FillColor);
    Content->Graphics.SetMeshDirty();
}

void UGGraph::DrawPolygon(float LineWidth, const FColor& LineColor, const FColor& FillColor, const TArray<FVector2D>& Points)
{
    FPolygonMesh& mesh = Content->Graphics.GetMeshFactory<FPolygonMesh>();
    mesh.LineWidth = LineWidth;
    mesh.LineColor = LineColor;
    mesh.Points.Reset();
    mesh.Points.Append(Points);
    mesh.FillColor.Reset();
    mesh.Colors.Reset();

    Content->Graphics.SetColor(FillColor);
    Content->Graphics.SetMeshDirty();
}

void UGGraph::DrawRegularPolygon(int32 Sides, float LineWidth, const FColor& LineColor, const FColor& FillColor, float ShapeRotation, const TArray<float>& Distances)
{
    FRegularPolygonMesh& mesh = Content->Graphics.GetMeshFactory<FRegularPolygonMesh>();
    mesh.Sides = Sides;
    mesh.LineWidth = LineWidth;
    mesh.CenterColor.Reset();
    mesh.LineColor = LineColor;
    mesh.FillColor.Reset();
    mesh.Rotation = ShapeRotation;
    mesh.Distances.Reset();
    mesh.Distances.Append(Distances);

    Content->Graphics.SetColor(FillColor);
    Content->Graphics.SetMeshDirty();
}

void UGGraph::Clear()
{
    Content->Graphics.SetMeshFactory(nullptr);
}

bool UGGraph::IsEmpty() const
{
    return !Content->Graphics.GetMeshFactory().IsValid();
}

IHitTest* UGGraph::GetHitArea() const
{
    const TSharedPtr<IMeshFactory>& Factory = Content->Graphics.GetMeshFactory();
    if (Factory.IsValid())
        return Factory->GetMeshHitTest();
    else
        return nullptr;
}

FNVariant UGGraph::GetProp(EObjectPropID PropID) const
{
    switch (PropID)
    {
    case EObjectPropID::Color:
        return FNVariant(Content->Graphics.GetColor());
    default:
        return UGObject::GetProp(PropID);
    }
}

void UGGraph::SetProp(EObjectPropID PropID, const FNVariant& InValue)
{
    switch (PropID)
    {
    case EObjectPropID::Color:
        SetColor(InValue.AsColor());
        break;
    default:
        UGObject::SetProp(PropID, InValue);
        break;
    }
}

void UGGraph::SetupBeforeAdd(FByteBuffer* Buffer, int32 BeginPos)
{
    UGObject::SetupBeforeAdd(Buffer, BeginPos);

    Buffer->Seek(BeginPos, 5);

    int32 type = Buffer->ReadByte();
    if (type != 0)
    {
        int32 lineWidth = Buffer->ReadInt();
        FColor lineColor = Buffer->ReadColor();
        FColor fillColor = Buffer->ReadColor();
        bool roundedRect = Buffer->ReadBool();
        FVector4 cornerRadius;
        if (roundedRect)
        {
            for (int32 i = 0; i < 4; i++)
                cornerRadius[i] = Buffer->ReadFloat();
        }

        if (type == 1)
        {
            if (roundedRect)
                DrawRoundRect(lineWidth, lineColor, fillColor, cornerRadius.X, cornerRadius.Y, cornerRadius.Z, cornerRadius.W);
            else
                DrawRect(lineWidth, lineColor, fillColor);
        }
        else if (type == 2)
            DrawEllipse(lineWidth, lineColor, fillColor);
        else if (type == 3)
        {
            int32 cnt = Buffer->ReadShort() / 2;
            TArray<FVector2D> points;
            for (int32 i = 0; i < cnt; i++)
            {
                float f1 = Buffer->ReadFloat();
                float f2 = Buffer->ReadFloat();
                points.Add(FVector2D(f1, f2));
            }

            DrawPolygon(lineWidth, lineColor, fillColor, points);
        }
        else if (type == 4)
        {
            int32 sides = Buffer->ReadShort();
            float startAngle = Buffer->ReadFloat();
            int32 cnt = Buffer->ReadShort();
            TArray<float> distances;
            if (cnt > 0)
            {
                for (int32 i = 0; i < cnt; i++)
                    distances.Add(Buffer->ReadFloat());
            }

            DrawRegularPolygon(sides, lineWidth, lineColor, fillColor, startAngle, distances);
        }
    }
}