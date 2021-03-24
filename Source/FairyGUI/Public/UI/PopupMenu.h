#pragma once

#include "GList.h"
#include "GButton.h"
#include "PopupMenu.generated.h"

UCLASS(BlueprintType)
class FAIRYGUI_API UPopupMenu : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "FairyGUI", meta = (WorldContext = "WorldContextObject"))
    static UPopupMenu* CreatePopupMenu(const FString& ResourceURL, UObject* WorldContextObject);

    static UPopupMenu* CreatePopupMenu(UObject* WorldContextObject) { return CreatePopupMenu("", WorldContextObject); }

    UPopupMenu();
    virtual ~UPopupMenu();

    UGButton* AddItem(const FString& Caption, FGUIEventDelegate Callback);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI", meta = (AutoCreateRefTerm = "Callback"))
    UGButton* AddItem(const FString& Caption, const FGUIEventDynDelegate& Callback);

    UGButton* AddItemAt(const FString& Caption, int32 index, FGUIEventDelegate Callback);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI", meta = (AutoCreateRefTerm = "Callback"))
    UGButton* AddItemAt(const FString& Caption, int32 index, const FGUIEventDynDelegate& Callback);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void AddSeperator();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    const FString& GetItemName(int32 Index) const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetItemText(const FString& Name, const FString& Caption);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetItemVisible(const FString& Name, bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetItemGrayed(const FString& Name, bool bGrayed);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetItemCheckable(const FString& Name, bool bCheckable);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetItemChecked(const FString& Name, bool bCheck);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsItemChecked(const FString& Name) const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool RemoveItem(const FString& Name);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void ClearItems();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 GetItemCount() const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGComponent* GetContentPane() const { return ContentPane; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGList* GetList() const { return List; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void Show(UGObject* AtObject, EPopupDirection Dir = EPopupDirection::Auto);

protected:
    void Create(const FString& ResourceURL);

    UPROPERTY(Transient)
    UGComponent* ContentPane;
    UGList* List;

private:
    void OnClickItem(UEventContext* Context);
    void OnAddedToStage(UEventContext* Context);

    static const FName ClickMenu;
};