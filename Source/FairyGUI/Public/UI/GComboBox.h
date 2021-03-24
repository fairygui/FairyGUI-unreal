#pragma once

#include "GComponent.h"
#include "GComboBox.generated.h"

class UGController;
class UGTextField;
class UGList;

UCLASS(BlueprintType, Blueprintable)
class FAIRYGUI_API UGComboBox : public UGComponent
{
    GENERATED_BODY()

public:
    UGComboBox();
    virtual ~UGComboBox();

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
    void SetTitleFontSize(int32 InFontSize);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    const FString& GetValue() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetValue(const FString& InValue);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 GetSelectedIndex() const { return SelectedIndex; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetSelectedIndex(int32 InIndex);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGController* GetSelectionController() const { return SelectionController; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetSelectionController(UGController* InController) { SelectionController = InController; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGObject* GetDropdown() const { return DropdownObject; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void Refresh();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    int32 VisibleItemCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    EPopupDirection PopupDirection;

    UGTextField* GetTextField() const;

    virtual FNVariant GetProp(EObjectPropID PropID) const override;
    virtual void SetProp(EObjectPropID PropID, const FNVariant& InValue) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    TArray<FString> Items;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    TArray<FString> Icons;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    TArray<FString> Values;

    UPROPERTY(BlueprintAssignable, Category = "FairyGUI|Event")
    FGUIEventDynMDelegate OnChanged;

protected:
    virtual void ConstructExtension(FByteBuffer* Buffer);
    virtual void SetupAfterAdd(FByteBuffer* Buffer, int32 BeginPos) override;

    virtual void HandleControllerChanged(UGController* Controller) override;
    virtual void HandleGrayedChanged() override;

    void SetState(const FString& InState);
    void SetCurrentState();
    void UpdateSelectionController();
    void UpdateDropdownList();
    void ShowDropdown();
    void RenderDropdownList();

    UPROPERTY(Transient)
    UGComponent* DropdownObject;
    UGObject* TitleObject;
    UGObject* IconObject;
    UGList* ListObject;
    UGController* SelectionController;

private:

    void OnClickItem(UEventContext* Context);
    void OnRollOverHandler(UEventContext* Context);
    void OnRollOutHandler(UEventContext* Context);
    void OnTouchBeginHandler(UEventContext* Context);
    void OnTouchEndHandler(UEventContext* Context);
    void OnPopupWinClosed(UEventContext* Context);

    bool bItemsUpdated;
    int32 SelectedIndex;
    UGController* ButtonController;
    bool bDown;
    bool bOver;
};