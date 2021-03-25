#pragma once

#include "GComponent.h"
#include "GSlider.generated.h"

UCLASS(BlueprintType, Blueprintable)
class FAIRYGUI_API UGSlider : public UGComponent
{
    GENERATED_BODY()

public:
    UGSlider();
    virtual ~UGSlider();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    EProgressTitleType GetTitleType() const { return TitleType; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetTitleType(EProgressTitleType InTitleType);

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
    bool GetWholeNumbers() const { return bWholeNumbers; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetWholeNumbers(bool bWholeNumbers);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    bool bChangeOnClick;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    bool bCanDrag;

    UPROPERTY(BlueprintAssignable, Category = "FairyGUI|Event")
    FGUIEventDynMDelegate OnChanged;

protected:
    virtual void HandleSizeChanged() override;
    virtual void ConstructExtension(FByteBuffer* Buffer);
    virtual void SetupAfterAdd(FByteBuffer* Buffer, int32 BeginPos) override;

    void Update();
    void UpdateWithPercent(float Percent, bool bManual);

private:
    void OnTouchBeginHandler(UEventContext* Context);
    void OnGripTouchBegin(UEventContext* Context);
    void OnGripTouchMove(UEventContext* Context);

    float Min;
    float Max;
    float Value;
    EProgressTitleType TitleType;
    bool bReverse;
    bool bWholeNumbers;

    UGObject* TitleObject;
    UGObject* BarObjectH;
    UGObject* BarObjectV;
    UGObject* GripObject;
    FVector2D BarMaxSize;
    FVector2D BarMaxSizeDelta;
    FVector2D BarStartPosition;
    FVector2D ClickPos;
    float ClickPercent;
};