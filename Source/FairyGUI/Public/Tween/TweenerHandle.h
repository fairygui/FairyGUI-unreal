#pragma once

#include "CoreMinimal.h"
#include "TweenerHandle.generated.h"

USTRUCT(BlueprintType)
struct FAIRYGUI_API FTweenerHandle
{
    GENERATED_USTRUCT_BODY()

public:
    FTweenerHandle()
        : Handle(0)
    {
    }

    bool IsValid() const
    {
        return Handle != 0;
    }

    void Invalidate()
    {
        Handle = 0;
    }

    bool operator==(const FTweenerHandle& Other) const
    {
        return Handle == Other.Handle;
    }

    bool operator!=(const FTweenerHandle& Other) const
    {
        return Handle != Other.Handle;
    }

    FString ToString() const
    {
        return FString::Printf(TEXT("%ull"), Handle);
    }

private:
    FORCEINLINE int32 GetIndex() const
    {
        return (int32)(Handle & (uint64)(MaxIndex - 1));
    }

    FORCEINLINE uint64 GetSerialNumber() const
    {
        return Handle >> IndexBits;
    }

    static const uint32 IndexBits = 24;
    static const uint32 SerialNumberBits = 40;
    static const int32  MaxIndex = (int32)1 << IndexBits;
    static const uint64 MaxSerialNumber = (uint64)1 << SerialNumberBits;

    void SetIndex(int32 Index)
    {
        Handle = (uint64)(uint32)Index;
    }

    void IncreaseSerialNumber()
    {
        int32 Index = (int32)(Handle & (uint64)(MaxIndex - 1));
        uint64 SerialNumber = Handle >> IndexBits;
        SerialNumber++;
        if (!ensureMsgf(SerialNumber != FTweenerHandle::MaxSerialNumber, TEXT("Tweener serial number has wrapped around!")))
        {
            SerialNumber = 0;
        }
        Handle = (SerialNumber << IndexBits) | (uint64)(uint32)Index;
    }

    uint64 Handle;
    friend class FTweenManager;
};
