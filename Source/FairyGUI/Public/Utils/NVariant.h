#pragma once

#include "CoreMinimal.h"
#include "Containers/Union.h"
#include "NVariant.generated.h"

USTRUCT(BlueprintType)
struct FAIRYGUI_API FNVariant
{
    GENERATED_USTRUCT_BODY()
public:
    FNVariant();
    FNVariant(const FNVariant& other);
    FNVariant(FNVariant&& other);
    ~FNVariant();

    explicit FNVariant(bool bValue);
    explicit FNVariant(int32 Value);
    explicit FNVariant(float Value);
    explicit FNVariant(const FString& Value);
    explicit FNVariant(const FColor& Value);
    explicit FNVariant(void* Value);

    FNVariant& operator= (const FNVariant& other);
    FNVariant& operator= (FNVariant&& other);

    FNVariant& operator= (bool bValue);
    FNVariant& operator= (int32 Value);
    FNVariant& operator= (float Value);
    FNVariant& operator= (const FString& Value);
    FNVariant& operator= (const FColor& Value);
    FNVariant& operator= (void* Value);

    bool AsBool() const
    {
        return As<bool>();
    }

    int32 AsInt() const
    {
        return As<int32>();
    }

    float AsFloat() const
    {
        return As<float>();
    }
    
    FString AsString() const
    {
        return As<FString>();
    }

    FColor AsColor() const
    {
        return As<FColor>();
    }

    UObject* AsUObject() const
    {
        return (UObject*)As<void *>();
    }

    template<typename DataType>
    DataType As() const
    {
        if (TheUnion.HasSubtype<DataType>())
            return TheUnion.GetSubtype<DataType>();
        else
            return DataType();
    }

    void Reset()
    {
        TheUnion.Reset();
    }

    static const FNVariant Null;

private:
    TUnion<bool, int32, float, FString, FColor, void*> TheUnion;
};