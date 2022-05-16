#include "Widgets/Mesh/PolygonMesh.h"

FPolygonMesh::FPolygonMesh() :
    LineWidth(0),
    LineColor(FColor::Black),
    bUsePercentPositions(false)
{
}

void FPolygonMesh::OnPopulateMesh(FVertexHelper& Helper)
{
    int32 numVertices = Points.Num();
    if (numVertices < 3)
        return;

    const FColor& color = FillColor.IsSet() ? FillColor.GetValue() : Helper.VertexColor;

    FVector2D Size = Helper.ContentRect.GetSize();
    bool useTexcoords = Texcoords.Num() >= numVertices;
    for (int32 i = 0; i < numVertices; i++)
    {
        FVector2D vec = Points[i];
        if (bUsePercentPositions)
            vec *= Size;

        if (useTexcoords)
        {
            FVector2D uv = FMath::Lerp(Helper.UVRect.Min, Helper.UVRect.Max, Texcoords[i]);
            Helper.AddVertex(vec, color, uv);
        }
        else
            Helper.AddVertex(vec, color);
    }

    // Algorithm "Ear clipping method" described here:
    // -> https://en.wikipedia.org/wiki/Polygon_triangulation
    //
    // Implementation inspired by:
    // -> http://polyk.ivank.net
    // -> Starling

    TArray<int32> RestIndices;
    for (int32 i = 0; i < numVertices; ++i)
        RestIndices.Add(i);

    int32 restIndexPos = 0;
    int32 numRestIndices = numVertices;

    while (numRestIndices > 3)
    {
        bool earFound = false;
        int32 i0 = RestIndices[restIndexPos % numRestIndices];
        int32 i1 = RestIndices[(restIndexPos + 1) % numRestIndices];
        int32 i2 = RestIndices[(restIndexPos + 2) % numRestIndices];

        const FVector2D& a = Points[i0];
        const FVector2D& b = Points[i1];
        const FVector2D& c = Points[i2];

        if ((a.Y - b.Y) * (c.X - b.X) + (b.X - a.X) * (c.Y - b.Y) >= 0)
        {
            earFound = true;
            for (int32 i = 3; i < numRestIndices; ++i)
            {
                int32 otherIndex = RestIndices[(restIndexPos + i) % numRestIndices];
                const FVector2D& p = Points[otherIndex];

                if (IsPointInTriangle(p, a, b, c))
                {
                    earFound = false;
                    break;
                }
            }
        }

        if (earFound)
        {
            Helper.AddTriangle(i0, i1, i2);
            RestIndices.RemoveAt((restIndexPos + 1) % numRestIndices);

            numRestIndices--;
            restIndexPos = 0;
        }
        else
        {
            restIndexPos++;
            if (restIndexPos == numRestIndices) break; // no more ears
        }
    }
    Helper.AddTriangle(RestIndices[0], RestIndices[1], RestIndices[2]);

    if (Colors.IsSet())
        Helper.RepeatColors(Colors.GetValue().GetData(), Colors.GetValue().Num(), 0, Helper.GetVertexCount());

    if (LineWidth > 0)
        DrawOutline(Helper);
}

void FPolygonMesh::DrawOutline(FVertexHelper& Helper)
{
    int32 numVertices = Points.Num();
    int32 k = Helper.GetVertexCount();
    int32 start = k - numVertices;
    for (int32 i = 0; i < numVertices; i++)
    {
        const FVector2D& p0 = FVector2D(Helper.Vertices[start + i].Position);
        FVector2D p1;
        if (i < numVertices - 1)
            p1 = FVector2D(Helper.Vertices[start + i + 1].Position);
        else
            p1 = FVector2D(Helper.Vertices[start].Position);

        FVector2D widthVector(p1.Y - p0.Y, p0.X - p1.X);
        widthVector.Normalize();

        Helper.AddVertex(p0 - widthVector * LineWidth * 0.5f, LineColor);
        Helper.AddVertex(p0 + widthVector * LineWidth * 0.5f, LineColor);
        Helper.AddVertex(p1 - widthVector * LineWidth * 0.5f, LineColor);
        Helper.AddVertex(p1 + widthVector * LineWidth * 0.5f, LineColor);

        k += 4;
        Helper.AddTriangle(k - 4, k - 3, k - 1);
        Helper.AddTriangle(k - 4, k - 1, k - 2);

        //joint
        if (i != 0)
        {
            Helper.AddTriangle(k - 6, k - 5, k - 3);
            Helper.AddTriangle(k - 6, k - 3, k - 4);
        }
        if (i == numVertices - 1)
        {
            start += numVertices;
            Helper.AddTriangle(k - 2, k - 1, start + 1);
            Helper.AddTriangle(k - 2, start + 1, start);
        }
    }
}

bool FPolygonMesh::IsPointInTriangle(const FVector2D& p, const FVector2D& a, const FVector2D& b, const FVector2D& c)
{
    // From Starling
    // This algorithm is described well in this article:
    // http://www.blackpawn.com/texts/pointinpoly/default.html

    float v0x = c.X - a.X;
    float v0y = c.Y - a.Y;
    float v1x = b.X - a.X;
    float v1y = b.Y - a.Y;
    float v2x = p.X - a.X;
    float v2y = p.Y - a.Y;

    float dot00 = v0x * v0x + v0y * v0y;
    float dot01 = v0x * v1x + v0y * v1y;
    float dot02 = v0x * v2x + v0y * v2y;
    float dot11 = v1x * v1x + v1y * v1y;
    float dot12 = v1x * v2x + v1y * v2y;

    float invDen = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDen;
    float v = (dot00 * dot12 - dot01 * dot02) * invDen;

    return (u >= 0) && (v >= 0) && (u + v < 1);
}

bool FPolygonMesh::HitTest(const FBox2D& ContentRect, const FVector2D& LayoutScaleMultiplier, const FVector2D& LocalPoint) const
{
    // Algorithm & implementation thankfully taken from:
    // -> http://alienryderflex.com/polygon/
    // inspired by Starling
    int32 len = Points.Num();
    int32 i;
    int32 j = len - 1;
    bool oddNodes = false;
    FVector2D ContentSize = ContentRect.GetSize();

    for (i = 0; i < len; ++i)
    {
        FVector2D vi = Points[i];
        FVector2D vj = Points[j];
        if (bUsePercentPositions)
        {
            vi *= ContentSize;
            vj *= ContentSize;
        }

        if (((vi.Y < LocalPoint.Y && vj.Y >= LocalPoint.Y) || (vj.Y < LocalPoint.Y && vi.Y >= LocalPoint.Y)) && (vi.X <= LocalPoint.X || vj.X <= LocalPoint.X))
        {
            if (vi.X + (LocalPoint.Y - vi.Y) / (vj.Y - vi.Y) * (vj.X - vi.X) < LocalPoint.X)
                oddNodes = !oddNodes;
        }

        j = i;
    }

    return oddNodes;
}