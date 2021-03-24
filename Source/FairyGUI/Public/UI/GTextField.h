#pragma once

#include "GObject.h"
#include "Widgets/NTextFormat.h"
#include "GTextField.generated.h"

UCLASS(BlueprintType)
class FAIRYGUI_API UGTextField : public UGObject
{
    GENERATED_BODY()

public:
    UGTextField();
    virtual ~UGTextField();

    virtual const FString& GetText() const override { return Text; }
    void SetText(const FString& InText) override;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsUBBEnabled() const { return bUBBEnabled; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    virtual void SetUBBEnabled(bool InEnabled);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    EAutoSizeType GetAutoSize() const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    virtual void SetAutoSize(EAutoSizeType InAutoSize);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    virtual bool IsSingleLine() const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    virtual void SetSingleLine(bool InSingleLine);

    UFUNCTION(BlueprintPure, Category = "FairyGUI")
    FNTextFormat& GetTextFormat();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetTextFormat(const FNTextFormat& InTextFormat);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void ApplyFormat();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    virtual FVector2D GetTextSize();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGTextField* SetVar(const FString& VarKey, const FString& VarValue);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void FlushVars();

    virtual FNVariant GetProp(EObjectPropID PropID) const override;
    virtual void SetProp(EObjectPropID PropID, const FNVariant& InValue) override;

    TOptional<TMap<FString, FString>> TemplateVars;

protected:
    virtual void SetupBeforeAdd(FByteBuffer* Buffer, int32 BeginPos) override;
    virtual void SetupAfterAdd(FByteBuffer* Buffer, int32 BeginPos) override;

    void UpdateSize();
    FString ParseTemplate(const FString& Template);

    FString Text;
    bool bUBBEnabled;
    bool bFormatApplied;
    bool bSupportHTML;

    TSharedPtr<class STextField> Content;
};