#pragma once

#include "GComponent.h"
#include "GProgressBar.generated.h"

UCLASS(BlueprintType, Blueprintable)
class FAIRYGUI_API UGProgressBar : public UGComponent
{
    GENERATED_BODY()

public:
    UGProgressBar();
    virtual ~UGProgressBar();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    EProgressTitleType GetTitleType() const { return TitleType; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetTitleType(EProgressTitleType InType);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    float GetMin() const { return Min; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetMin(float InMin);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    float GetMax() const { return Max; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetMax(float InMax);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    float GetValue() const { return Value; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetValue(float InValue);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void TweenValue(float InValue, float Duration);

    void Update(float NewValue);

protected:
    virtual void HandleSizeChanged() override;
    virtual void ConstructExtension(FByteBuffer* Buffer);
    virtual void SetupAfterAdd(FByteBuffer* Buffer, int32 BeginPos) override;

    bool SetFillAmount(UGObject* Bar, float Amount);

private:
    float Min;
    float Max;
    float Value;
    EProgressTitleType TitleType;
    bool bReverse;
    FTweenerHandle TweenHandle;

    UGObject* TitleObject;
    UGObject* BarObjectH;
    UGObject* BarObjectV;
    FVector2D BarMaxSize;
    FVector2D BarMaxSizeDelta;
    FVector2D BarStartPosition;
};