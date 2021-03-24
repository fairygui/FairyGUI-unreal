#pragma once

#include "GObject.h"
#include "GLoader.generated.h"

UCLASS(BlueprintType, Blueprintable)
class FAIRYGUI_API UGLoader : public UGObject
{
    GENERATED_BODY()

public:
    UGLoader();
    virtual ~UGLoader();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    const FString& GetURL() const { return URL; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetURL(const FString& InURL);

    virtual const FString& GetIcon() const override { return URL; }
    virtual void SetIcon(const FString& InIcon) override { SetURL(InIcon); }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    EAlignType GetAlign() const { return Align; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetAlign(EAlignType InAlign);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    EVerticalAlignType GetVerticalAlign() const { return VerticalAlign; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetVerticalAlign(EVerticalAlignType InVerticalAlign);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool GetAutoSize() const { return bAutoSize; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetAutoSize(bool bInAutoSize);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    ELoaderFillType GetFill() const { return Fill; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetFill(ELoaderFillType InFillType);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsShrinkOnly() const { return bShrinkOnly; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetShrinkOnly(bool bInShrinkOnly);

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

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsPlaying() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetPlaying(bool bInPlaying);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 GetFrame() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetFrame(int32 InFrame);

    virtual FNVariant GetProp(EObjectPropID PropID) const;
    virtual void SetProp(EObjectPropID PropID, const FNVariant& InValue);

protected:
    virtual void HandleSizeChanged() override;
    virtual void SetupBeforeAdd(FByteBuffer* Buffer, int32 BeginPos) override;

    void LoadContent();
    void ClearContent();
    void LoadFromPackage(const FString& ItemURL);
    void LoadExternal();
    void OnExternalLoaded(FString LoadingURL);
    void UpdateLayout();
    void SetErrorState();

    TSharedPtr<FSoftObjectPath> SoftObjectPath;

private:
    TSharedPtr<class SContainer> Container;
    TSharedPtr<class SMovieClip> Content;
    UPROPERTY(Transient)
    UGObject* Content2;
    TSharedPtr<FPackageItem> ContentItem;
    FString URL;
    ELoaderFillType Fill;
    EAlignType Align;
    EVerticalAlignType VerticalAlign;
    uint8 bShowErrorSign : 1;
    uint8 bShrinkOnly : 1;
    uint8 bAutoSize : 1;
    uint8 bUpdatingLayout : 1;
};