#pragma once

#include "GObject.h"
#include "GLoader3D.generated.h"

UCLASS(BlueprintType, Blueprintable)
class FAIRYGUI_API UGLoader3D : public UGObject
{
    GENERATED_BODY()

public:
    UGLoader3D();
    virtual ~UGLoader3D();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    const FString& GetURL() const { return URL; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetURL(const FString& InURL);

    virtual const FString& GetIcon() const override { return URL; }
    virtual void SetIcon(const FString& InIcon) override { SetURL(InIcon); }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    FColor GetColor() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetColor(const FColor& InColor);

    virtual FNVariant GetProp(EObjectPropID PropID) const;
    virtual void SetProp(EObjectPropID PropID, const FNVariant& InValue);

protected:
    virtual void HandleSizeChanged() override;
    virtual void SetupBeforeAdd(FByteBuffer* Buffer, int32 BeginPos) override;

    void LoadContent();
    void ClearContent();
    void LoadFromPackage(const FString& ItemURL);
    void LoadExternal();
    void UpdateLayout();
    void SetErrorState();

private:
    TSharedPtr<class SFImage> Content;
    TSharedPtr<FPackageItem> ContentItem;
    FString URL;
    ELoaderFillType Fill;
    EAlignType Align;
    EVerticalAlignType VerticalAlign;
    bool bShowErrorSign;
    bool bShrinkOnly;
    bool bAutoSize;
    bool bUpdatingLayout;
};