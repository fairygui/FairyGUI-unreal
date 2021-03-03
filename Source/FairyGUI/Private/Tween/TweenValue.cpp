#include "Tween/TweenValue.h"

FTweenValue::FTweenValue() :X(0), Y(0), Z(0), W(0), D(0)
{
}

FVector2D FTweenValue::GetVec2() const
{
    return FVector2D(X, Y);
}

void FTweenValue::SetVec2(const FVector2D & Value)
{
    X = Value.X;
    Y = Value.Y;
}

FVector FTweenValue::GetVec3() const
{
    return FVector(X, Y, Z);
}

void FTweenValue::SetVec3(const FVector & Value)
{
    X = Value.X;
    Y = Value.Y;
    Z = Value.Z;
}

FVector4 FTweenValue::GetVec4() const
{
    return FVector4(X, Y, Z, W);
}

void FTweenValue::SetVec4(const FVector4 & Value)
{
    X = Value.X;
    Y = Value.Y;
    Z = Value.Z;
    W = Value.W;
}

FColor FTweenValue::GetColor() const
{
    return FColor(X * 255, Y * 255, Z * 255, W * 255);
}

void FTweenValue::SetColor(const FColor & Value)
{
    X = Value.R / 255.f;
    Y = Value.G / 255.f;
    Z = Value.B / 255.f;
    W = Value.A / 255.f;
}

float FTweenValue::operator[](int32 Index) const
{
    verifyf(Index < 4, TEXT("Index out of bounds: %d"), Index);

    switch (Index)
    {
    case 0:
        return X;
    case 1:
        return Y;
    case 2:
        return Z;
    case 3:
        return W;
    default:
        return X;
    }
}

float& FTweenValue::operator[](int32 Index)
{
    verifyf(Index < 4, TEXT("Index out of bounds: %d"), Index);

    switch (Index)
    {
    case 0:
        return X;
    case 1:
        return Y;
    case 2:
        return Z;
    case 3:
        return W;
    default:
        return X;
    }
}

void FTweenValue::Reset()
{
    X = Y = Z = W = 0;
    D = 0;
}