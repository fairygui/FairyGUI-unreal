#pragma once

#include "GComponent.h"
#include "GScrollBar.generated.h"

class UScrollPane;

UCLASS(BlueprintType, Blueprintable)
class FAIRYGUI_API UGScrollBar : public UGComponent
{
    GENERATED_BODY()

public:
    UGScrollBar();
    virtual ~UGScrollBar();

    void SetScrollPane(UScrollPane* Target, bool bVertical);
    void SetDisplayPerc(float Value);
    void SetScrollPerc(float Value);
    float GetMinSize();

    bool bGripDragging;

protected:
    virtual void ConstructExtension(FByteBuffer* Buffer);

private:
    void OnTouchBeginHandler(UEventContext* Context);
    void OnGripTouchBegin(UEventContext* Context);
    void OnGripTouchMove(UEventContext* Context);
    void OnGripTouchEnd(UEventContext* Context);
    void OnArrowButton1Click(UEventContext* Context);
    void OnArrowButton2Click(UEventContext* Context);

    UGObject* GripObject;
    UGObject* ArrowButton1;
    UGObject* ArrowButton2;
    UGObject* BarObject;
    UScrollPane* Target;

    bool bVertical;
    float ScrollPerc;
    bool bFixedGripSize;

    FVector2D DragOffset;
};