#pragma once

#include "GComponent.h"
#include "GLabel.generated.h"

class UGTextField;

UCLASS(BlueprintType, Blueprintable)
class FAIRYGUI_API UGLabel : public UGComponent
{
    GENERATED_BODY()

public:
    UGLabel();
    virtual ~UGLabel();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    const FString& GetTitle() const { return GetText(); }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetTitle(const FString& InTitle) { SetText(InTitle); };

    virtual const FString& GetText() const override;
    virtual void SetText(const FString& InText) override;

    virtual const FString& GetIcon() const override;
    virtual void SetIcon(const FString& InIcon) override;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    FColor GetTitleColor() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetTitleColor(const FColor& InColor);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 GetTitleFontSize() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetTitleFontSize(int32 value);

    UGTextField* GetTextField() const;

    virtual FNVariant GetProp(EObjectPropID PropID) const override;
    virtual void SetProp(EObjectPropID PropID, const FNVariant& InValue) override;

protected:
    virtual void ConstructExtension(FByteBuffer* Buffer);
    virtual void SetupAfterAdd(FByteBuffer* Buffer, int32 BeginPos) override;

    UGObject* TitleObject;
    UGObject* IconObject;
};