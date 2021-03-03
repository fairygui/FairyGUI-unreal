#pragma once

#include "CoreMinimal.h"

struct FAIRYGUI_API FGPathPoint
{
    enum class ECurveType
    {
        CRSpline,
        Bezier,
        CubicBezier,
        Straight
    };

    FVector Pos;
    FVector Control1;
    FVector Control2;
    ECurveType CurveType;

    FGPathPoint(const FVector& InPos);
    FGPathPoint(const FVector& InPos, const FVector& InControl);
    FGPathPoint(const FVector& InPos, const FVector& InControl1, const FVector& InControl2);
    FGPathPoint(const FVector& InPos, ECurveType InCurveType);
};

class FAIRYGUI_API FGPath
{
public:
    FGPath();
    void Create(const FGPathPoint* InPoints, int32 Count);
    void Clear();
    FVector GetPointAt(float Time);

    float GetLength() { return FullLength; }
    int32 GetSegmentCount() { return Segments.Num(); }
    float GetSegmentLength(int32 SegmentIndex);
    void GetPointsInSegment(int32 SegmentIndex, float t0, float t1,
        TArray<FVector>& OutPoints, TArray<float>* OutTimeArray = nullptr, float PointDensity = 0.1f);
    void GetAllPoints(TArray<FVector>& OutPoints, float PointDensity = 0.1f);

    struct FSegment
    {
        FGPathPoint::ECurveType Type;
        float Length;
        int32 PointStart;
        int32 PointCount;
    };

private:
    void CreateSplineSegment(TArray<FVector>& InOutSplinePoints);
    FVector OnCRSplineCurve(int32 PointStart, int32 PointCount, float Time);
    FVector OnBezierCurve(int32 PointStart, int32 PointCount, float Time);

    TArray<FSegment> Segments;
    TArray<FVector> Points;
    float FullLength;
};
