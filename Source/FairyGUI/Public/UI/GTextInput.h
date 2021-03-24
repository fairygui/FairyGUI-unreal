#pragma once

#include "GObject.h"
#include "Widgets/NTextFormat.h"
#include "GTextInput.generated.h"

UCLASS(BlueprintType)
class FAIRYGUI_API UGTextInput : public UGObject
{
    GENERATED_BODY()

public:
    UGTextInput();
    virtual ~UGTextInput();

    virtual const FString& GetText() const override { return Text; }
    void SetText(const FString& InText) override;

    TSharedRef<SMultiLineEditableText> GetInputWidget() const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsSingleLine() const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetSingleLine(bool InSingleLine);

    UFUNCTION(BlueprintPure, Category = "FairyGUI")
    FNTextFormat& GetTextFormat() { return TextFormat; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetTextFormat(const FNTextFormat& InTextFormat);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void ApplyFormat();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetPrompt(const FString& InPrompt);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetPassword(bool bInPassword);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetKeyboardType(int32 InKeyboardType);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetMaxLength(int32 InMaxLength);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetRestrict(const FString& InRestrict);

    UPROPERTY(BlueprintAssignable, Category = "FairyGUI|Event")
    FGUIEventDynMDelegate OnSubmit;

    virtual FNVariant GetProp(EObjectPropID PropID) const override;
    virtual void SetProp(EObjectPropID PropID, const FNVariant& InValue) override;

    void NotifyTextChanged(const FText& InText);

protected:
    virtual void SetupBeforeAdd(FByteBuffer* Buffer, int32 BeginPos) override;
    virtual void SetupAfterAdd(FByteBuffer* Buffer, int32 BeginPos) override;

    FString Text;
    bool bFormatApplied;
    FNTextFormat TextFormat;

    TSharedPtr<class STextInput> Content;
};