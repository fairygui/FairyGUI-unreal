#pragma once

#include "GObject.h"
#include "GImage.generated.h"

UCLASS(BlueprintType)
class FAIRYGUI_API UGImage : public UGObject
{
    GENERATED_BODY()

public:
    UGImage();
    virtual ~UGImage();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    EFlipType GetFlip() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetFlip(EFlipType InFlip);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    FColor GetColor() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetColor(const FColor& InColor);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    EFillMethod GetFillMethod() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetFillMethod(EFillMethod Method);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 GetFillOrigin() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetFillOrigin(int32 Origin);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsFillClockwise() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetFillClockwise(bool bClockwise);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    float GetFillAmount() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetFillAmount(float Amount);

    virtual void ConstructFromResource() override;

    virtual FNVariant GetProp(EObjectPropID PropID) const override;
    virtual void SetProp(EObjectPropID PropID, const FNVariant& InValue) override;

protected:
    virtual void SetupBeforeAdd(FByteBuffer* Buffer, int32 BeginPos) override;

private:
    TSharedPtr<class SFImage> Content;
};