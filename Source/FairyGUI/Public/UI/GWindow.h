#pragma once

#include "GComponent.h"
#include "GWindow.generated.h"

class FAIRYGUI_API IUISource
{
public:
    virtual const FString& GetFileName() = 0;
    virtual void SetFileName(const FString& InFileName) = 0;
    virtual bool IsLoaded() = 0;
    virtual void Load(FSimpleDelegate Callback) = 0;
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FWindowDynDelegate, class UGWindow*, Window);

UCLASS(BlueprintType, Blueprintable)
class FAIRYGUI_API UGWindow : public UGComponent
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "FairyGUI", meta = (WorldContext = "WorldContextObject"))
    static UGWindow* CreateWindow(const FString& PackageName, const FString& ResourceName, UObject* WorldContextObject);

    UGWindow();
    virtual ~UGWindow();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void Show();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void Hide();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void HideImmediately();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void ToggleStatus();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void BringToFront();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsShowing() const { return Parent.IsValid(); }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsTop() const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsModal() const { return bModal; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetModal(bool bInModal) { bModal = bInModal; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void ShowModalWait(int32 InRequestingCmd = 0);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool CloseModalWait(int32 InRequestingCmd = 0);

    void Init();
    void AddUISource(TSharedPtr<IUISource> UISource);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGComponent* GetContentPane() const { return ContentPane; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetContentPane(UGComponent* Obj);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGComponent* GetFrame() const { return FrameObject; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGObject* GetCloseButton() const { return CloseButton; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetCloseButton(UGObject* Obj);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGObject* GetDragArea() const { return DragArea; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetDragArea(UGObject* Obj);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGObject* GetContentArea() const { return ContentArea; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetContentArea(UGObject* Obj) { ContentArea = Obj; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGObject* GetModalWaitingPane() const { return ModalWaitPane; }

    virtual void OnShown();
    virtual void OnHide();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    bool bBringToFontOnClick;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    FWindowDynDelegate InitCallback;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    FWindowDynDelegate ShownCallback;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    FWindowDynDelegate HideCallback;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    FWindowDynDelegate ShowingCallback;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    FWindowDynDelegate HidingCallback;

protected:
    virtual void OnInit();
    virtual void DoShowAnimation();
    virtual void DoHideAnimation();

    UFUNCTION()
    void CloseEventHandler(UEventContext* Context);

    UPROPERTY(Transient)
    UGComponent* ContentPane;

private:
    void LayoutModalWaitPane();
    void OnUILoadComplete();
    void InternalInit();

    void OnTouchBeginHandler(UEventContext* Context);
    UFUNCTION()
    void OnDragStartHandler(UEventContext* Context);
    void OnAddedToStageHandler(UEventContext* Context);
    void OnRemovedFromStageHandler(UEventContext* Context);

    UPROPERTY(Transient)
    UGObject* ModalWaitPane;
    UPROPERTY(Transient)
    UGComponent* FrameObject;
    UPROPERTY(Transient)
    UGObject* CloseButton;
    UPROPERTY(Transient)
    UGObject* DragArea;
    UPROPERTY(Transient)
    UGObject* ContentArea;

    int32 RequestingCmd;
    TArray<TSharedPtr<IUISource>> UISources;
    uint8 bModal : 1;
    uint8 bInited : 1;
    uint8 bLoading : 1;
};