#pragma once

#include "CoreMinimal.h"
#include "TweenValue.generated.h"

USTRUCT(BlueprintType)
struct FAIRYGUI_API FTweenValue
{
    GENERATED_USTRUCT_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    float X;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    float Y;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    float Z;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    float W;

    double D;

    FTweenValue();

    FVector2D GetVec2() const;
    void SetVec2(const FVector2D& Value);
    FVector GetVec3() const;
    void SetVec3(const FVector& Value);
    FVector4 GetVec4() const;
    void SetVec4(const FVector4& Value);
    FColor GetColor() const;
    void SetColor(const FColor& Color);
    float operator[] (int32 Index) const;
    float& operator[] (int32 Index);
    void Reset();
};