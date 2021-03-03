#include "Tween/GPath.h"

FGPathPoint::FGPathPoint(const FVector& InPos)
{
    Pos = InPos;
    Control1 = FVector::ZeroVector;
    Control2 = FVector::ZeroVector;
    CurveType = ECurveType::CRSpline;
}

FGPathPoint::FGPathPoint(const FVector& InPos, const FVector& InControl)
{
    Pos = InPos;
    Control1 = InControl;
    Control2 = FVector::ZeroVector;
    CurveType = ECurveType::Bezier;
}

FGPathPoint::FGPathPoint(const FVector& InPos, const FVector& InControl1, const FVector& InControl2)
{
    Pos = InPos;
    Control1 = InControl1;
    Control2 = InControl2;
    CurveType = ECurveType::CubicBezier;
}

FGPathPoint::FGPathPoint(const FVector& InPos, ECurveType InCurveType)
{
    Pos = InPos;
    Control1 = FVector::ZeroVector;
    Control2 = FVector::ZeroVector;
    CurveType = InCurveType;
}

FGPath::FGPath()
    : FullLength(0)
{
}

void FGPath::Create(const FGPathPoint* InPoints, int32 Count)
{
    Segments.Reset();
    Points.Reset();
    FullLength = 0;

    if (Count == 0)
        return;

    TArray<FVector> SplinePoints;
    const FGPathPoint* prev = InPoints;
    if (prev->CurveType == FGPathPoint::ECurveType::CRSpline)
        SplinePoints.Add(prev->Pos);

    for (int32 i = 1; i < Count; i++)
    {
        const FGPathPoint* current = InPoints + i;

        if (prev->CurveType != FGPathPoint::ECurveType::CRSpline)
        {
            FSegment seg;
            seg.Type = prev->CurveType;
            seg.PointStart = Points.Num();
            if (prev->CurveType == FGPathPoint::ECurveType::Straight)
            {
                seg.PointCount = 2;
                Points.Add(prev->Pos);
                Points.Add(current->Pos);
            }
            else if (prev->CurveType == FGPathPoint::ECurveType::Bezier)
            {
                seg.PointCount = 3;
                Points.Add(prev->Pos);
                Points.Add(current->Pos);
                Points.Add(prev->Control1);
            }
            else if (prev->CurveType == FGPathPoint::ECurveType::CubicBezier)
            {
                seg.PointCount = 4;
                Points.Add(prev->Pos);
                Points.Add(current->Pos);
                Points.Add(prev->Control1);
                Points.Add(prev->Control2);
            }
            seg.Length = FVector::Dist(prev->Pos, current->Pos);
            FullLength += seg.Length;
            Segments.Add(MoveTemp(seg));
        }

        if (current->CurveType != FGPathPoint::ECurveType::CRSpline)
        {
            if (SplinePoints.Num() > 0)
            {
                SplinePoints.Add(current->Pos);
                CreateSplineSegment(SplinePoints);
            }
        }
        else
            SplinePoints.Add(current->Pos);

        prev = current;
    }

    if (SplinePoints.Num() > 1)
        CreateSplineSegment(SplinePoints);
}

void FGPath::CreateSplineSegment(TArray<FVector>& SplinePoints)
{
    int32 cnt = SplinePoints.Num();
    SplinePoints.Insert(SplinePoints[0], 0);
    SplinePoints.Add(SplinePoints[cnt]);
    SplinePoints.Add(SplinePoints[cnt]);
    cnt += 3;

    FSegment seg;
    seg.Type = FGPathPoint::ECurveType::CRSpline;
    seg.PointStart = Points.Num();
    seg.PointCount = cnt;
    for (auto& it : SplinePoints)
        Points.Add(it);

    seg.Length = 0;
    for (int32 i = 1; i < cnt; i++)
        seg.Length += FVector::Dist(SplinePoints[i - 1], SplinePoints[i]);
    FullLength += seg.Length;
    Segments.Add(MoveTemp(seg));
    SplinePoints.Reset();
}

void FGPath::Clear()
{
    Segments.Reset();
    Points.Reset();
}

FVector FGPath::GetPointAt(float Time)
{
    Time = FMath::Clamp<float>(Time, 0, 1);
    int32 cnt = Segments.Num();
    if (cnt == 0)
        return FVector::ZeroVector;

    if (Time == 1)
    {
        const FSegment& seg = Segments[cnt - 1];

        if (seg.Type == FGPathPoint::ECurveType::Straight)
            return FMath::Lerp(Points[seg.PointStart], Points[seg.PointStart + 1], Time);
        else if (seg.Type == FGPathPoint::ECurveType::Bezier || seg.Type == FGPathPoint::ECurveType::CubicBezier)
            return OnBezierCurve(seg.PointStart, seg.PointCount, Time);
        else
            return OnCRSplineCurve(seg.PointStart, seg.PointCount, Time);
    }

    float len = Time * FullLength;
    FVector pt;
    for (int32 i = 0; i < cnt; i++)
    {
        const FSegment& seg = Segments[i];

        len -= seg.Length;
        if (len < 0)
        {
            Time = 1 + len / seg.Length;

            if (seg.Type == FGPathPoint::ECurveType::Straight)
                pt = FMath::Lerp(Points[seg.PointStart], Points[seg.PointStart + 1], Time);
            else if (seg.Type == FGPathPoint::ECurveType::Bezier || seg.Type == FGPathPoint::ECurveType::CubicBezier)
                pt = OnBezierCurve(seg.PointStart, seg.PointCount, Time);
            else
                pt = OnCRSplineCurve(seg.PointStart, seg.PointCount, Time);

            break;
        }
    }

    return pt;
}

float FGPath::GetSegmentLength(int32 SegmentIndex)
{
    return Segments[SegmentIndex].Length;
}

void FGPath::GetPointsInSegment(int32 SegmentIndex, float t0, float t1,
    TArray<FVector>& OutPoints, TArray<float>* OutTimeArray, float PointDensity)
{
    if (OutTimeArray != nullptr)
        OutTimeArray->Add(t0);
    const FSegment& seg = Segments[SegmentIndex];
    if (seg.Type == FGPathPoint::ECurveType::Straight)
    {
        OutPoints.Add(FMath::Lerp(Points[seg.PointStart], Points[seg.PointStart + 1], t0));
        OutPoints.Add(FMath::Lerp(Points[seg.PointStart], Points[seg.PointStart + 1], t1));
    }
    else if (seg.Type == FGPathPoint::ECurveType::Bezier || seg.Type == FGPathPoint::ECurveType::CubicBezier)
    {
        OutPoints.Add(OnBezierCurve(seg.PointStart, seg.PointCount, t0));
        int32 SmoothAmount = FMath::Min(FMath::FloorToInt(seg.Length * PointDensity), 50);
        for (int32 j = 0; j <= SmoothAmount; j++)
        {
            float t = (float)j / SmoothAmount;
            if (t > t0 && t < t1)
            {
                OutPoints.Add(OnBezierCurve(seg.PointStart, seg.PointCount, t));
                if (OutTimeArray != nullptr)
                    OutTimeArray->Add(t);
            }
        }
        OutPoints.Add(OnBezierCurve(seg.PointStart, seg.PointCount, t1));
    }
    else
    {
        OutPoints.Add(OnCRSplineCurve(seg.PointStart, seg.PointCount, t0));
        int32 SmoothAmount = FMath::Min(FMath::FloorToInt(seg.Length * PointDensity), 50);
        for (int32 j = 0; j <= SmoothAmount; j++)
        {
            float t = (float)j / SmoothAmount;
            if (t > t0 && t < t1)
            {
                OutPoints.Add(OnCRSplineCurve(seg.PointStart, seg.PointCount, t));
                if (OutTimeArray != nullptr)
                    OutTimeArray->Add(t);
            }
        }
        OutPoints.Add(OnCRSplineCurve(seg.PointStart, seg.PointCount, t1));
    }

    if (OutTimeArray != nullptr)
        OutTimeArray->Add(t1);
}

void FGPath::GetAllPoints(TArray<FVector>& OutPoints, float PointDensity)
{
    int32 cnt = Segments.Num();
    for (int32 i = 0; i < cnt; i++)
        GetPointsInSegment(i, 0, 1, OutPoints, nullptr, PointDensity);
}

static float repeat(float t, float length)
{
    return t - FMath::FloorToFloat(t / length) * length;
}

FVector FGPath::OnCRSplineCurve(int32 PointStart, int32 PointCount, float Time)
{
    int32 adjustedIndex = FMath::FloorToFloat(Time * (PointCount - 4)) + PointStart; //Since the equation works with 4 points, we adjust the starting point depending on t to return a point on the specific segment

    FVector result;

    FVector p0 = Points[adjustedIndex];
    FVector p1 = Points[adjustedIndex + 1];
    FVector p2 = Points[adjustedIndex + 2];
    FVector p3 = Points[adjustedIndex + 3];

    float adjustedT = (Time == 1.f) ? 1.f : repeat(Time * (PointCount - 4), 1.f); // Then we adjust t to be that value on that new piece of segment... for t == 1f don't use repeat (that would return 0f);

    float t0 = ((-adjustedT + 2.f) * adjustedT - 1.f) * adjustedT * 0.5f;
    float t1 = (((3.f * adjustedT - 5.f) * adjustedT) * adjustedT + 2.f) * 0.5f;
    float t2 = ((-3.f * adjustedT + 4.f) * adjustedT + 1.f) * adjustedT * 0.5f;
    float t3 = ((adjustedT - 1.f) * adjustedT * adjustedT) * 0.5f;

    result.X = p0.X * t0 + p1.X * t1 + p2.X * t2 + p3.X * t3;
    result.Y = p0.Y * t0 + p1.Y * t1 + p2.Y * t2 + p3.Y * t3;
    result.Z = p0.Z * t0 + p1.Z * t1 + p2.Z * t2 + p3.Z * t3;

    return result;
}

FVector FGPath::OnBezierCurve(int32 PointStart, int32 PointCount, float Time)
{
    float t2 = 1.0f - Time;
    FVector p0 = Points[PointStart];
    FVector p1 = Points[PointStart + 1];
    FVector cp0 = Points[PointStart + 2];

    if (PointCount == 4)
    {
        FVector cp1 = Points[PointStart + 3];
        return t2 * t2 * t2 * p0 + 3.f * t2 * t2 * Time * cp0 + 3.f * t2 * Time * Time * cp1 + Time * Time * Time * p1;
    }
    else
        return t2 * t2 * p0 + 2.f * t2 * Time * cp0 + Time * Time * p1;
}