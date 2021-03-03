#include "Utils/NVariant.h"

const FNVariant FNVariant::Null;

FNVariant::FNVariant()
{
}

FNVariant::~FNVariant()
{
}

FNVariant::FNVariant(const FNVariant& other)
{
    *this = other;
}

FNVariant::FNVariant(FNVariant&& other)
{
    *this = MoveTemp(other);
}

FNVariant::FNVariant(bool bValue)
{
    TheUnion.SetSubtype<bool>(bValue);
}

FNVariant::FNVariant(int32 Value)
{
    TheUnion.SetSubtype<int32>(Value);
}

FNVariant::FNVariant(float Value)
{
    TheUnion.SetSubtype<float>(Value);
}

FNVariant::FNVariant(const FString& Value)
{
    TheUnion.SetSubtype<FString>(Value);
}

FNVariant::FNVariant(const FColor& Value)
{
    TheUnion.SetSubtype<FColor>(Value);
}

FNVariant::FNVariant(void* Value)
{
    TheUnion.SetSubtype<void*>(Value);
}

FNVariant& FNVariant::operator= (const FNVariant& other)
{
    if (this != &other)
        TheUnion = other.TheUnion;
    return *this;
}

FNVariant& FNVariant::operator= (FNVariant&& other)
{
    if (this != &other)
        TheUnion = other.TheUnion;
    return *this;
}

FNVariant& FNVariant::operator= (bool bValue)
{
    TheUnion.SetSubtype<bool>(bValue);
    return *this;
}

FNVariant& FNVariant::operator= (int32 Value)
{
    TheUnion.SetSubtype<int32>(Value);
    return *this;
}

FNVariant& FNVariant::operator= (float Value)
{
    TheUnion.SetSubtype<float>(Value);
    return *this;
}

FNVariant& FNVariant::operator= (const FString& Value)
{
    TheUnion.SetSubtype<FString>(Value);
    return *this;
}

FNVariant& FNVariant::operator= (const FColor& Value)
{
    TheUnion.SetSubtype<FColor>(Value);
    return *this;
}

FNVariant& FNVariant::operator= (void* Value)
{
    TheUnion.SetSubtype<void*>(Value);
    return *this;
}
