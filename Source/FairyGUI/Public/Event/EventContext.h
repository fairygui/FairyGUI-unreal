#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Input/Events.h"
#include "EventTypes.h"
#include "Utils/NVariant.h"
#include "EventContext.generated.h"

class UGObject;
class SDisplayObject;

UCLASS(BlueprintType)
class FAIRYGUI_API UEventContext : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGObject* GetSender() const
    {
        return Sender;
    }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGObject* GetInitiator() const
    {
        return Initiator;
    }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    const FName& GetType() const
    {
        return Type;
    }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    const FVector2D& GetPointerPosition() const
    {
        return PointerEvent->GetScreenSpacePosition();
    }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 GetPointerIndex() const
    {
        return (int32)PointerEvent->GetPointerIndex();
    }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 GetUserIndex() const
    {
        return PointerEvent->GetUserIndex();
    }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    float GetWheelDelta() const
    {
        return PointerEvent->GetWheelDelta();
    }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    FKey GetMouseButton() const
    {
        if (PointerEvent->IsTouchEvent())
            return EKeys::LeftMouseButton;
        else
            return PointerEvent->GetEffectingButton();
    }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    FPointerEvent& GetPointerEvent() const
    {
        return *PointerEvent;
    }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    FKeyEvent& GetKeyEvent() const
    {
        return *KeyEvent;
    }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 GetClickCount() const
    {
        return ClickCount;
    }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsDoubleClick() const
    {
        return ClickCount == 2;
    }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    const FNVariant& GetData() const
    {
        return Data;
    }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void StopPropagation()
    {
        bStopped = true;
    }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsPropagationStopped() const
    {
        return bStopped;
    }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void PreventDefault()
    {
        bDefaultPrevented = true;
    }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsDefaultPrevented() const
    {
        return bDefaultPrevented;
    }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void CaptureTouch()
    {
        bIsMouseCaptor = true;
    }

private:
    UGObject* Sender;
    UGObject* Initiator;
    FName Type;
    bool bStopped;
    bool bDefaultPrevented;
    bool bIsMouseCaptor;
    FPointerEvent* PointerEvent;
    FKeyEvent* KeyEvent;
    int32 ClickCount;
    FNVariant Data;

    friend class UFairyApplication;
};

DECLARE_DELEGATE_OneParam(FGUIEventDelegate, UEventContext*);
DECLARE_MULTICAST_DELEGATE_OneParam(FGUIEventMDelegate, UEventContext*);

DECLARE_DYNAMIC_DELEGATE_OneParam(FGUIEventDynDelegate, UEventContext*, EventContext);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGUIEventDynMDelegate, UEventContext*, EventContext);

DECLARE_DYNAMIC_DELEGATE(FSimpleDynDelegate);