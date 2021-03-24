#include "UI/GWindow.h"
#include "UI/GGraph.h"
#include "UI/GRoot.h"
#include "UI/UIPackage.h"

UGWindow* UGWindow::CreateWindow(const FString& PackageName, const FString& ResourceName, UObject* WorldContextObject)
{
    UGObject* ContentPane = UUIPackage::CreateObject(PackageName, ResourceName, WorldContextObject);
    verifyf(ContentPane->IsA<UGComponent>(), TEXT("Window content should be a component"));
    UGWindow* Window = NewObject<UGWindow>(WorldContextObject);
    Window->SetContentPane(Cast<UGComponent>(ContentPane));

    return Window;
}

UGWindow::UGWindow()
{
    bBringToFontOnClick = FUIConfig::Config.BringWindowToFrontOnClick;

    On(FUIEvents::AddedToStage).AddUObject(this, &UGWindow::OnAddedToStageHandler);
    On(FUIEvents::RemovedFromStage).AddUObject(this, &UGWindow::OnRemovedFromStageHandler);
    On(FUIEvents::TouchBegin).AddUObject(this, &UGWindow::OnTouchBeginHandler);
}

UGWindow::~UGWindow()
{
}

void UGWindow::SetContentPane(UGComponent* Obj)
{
    if (ContentPane != Obj)
    {
        if (ContentPane != nullptr)
        {
            RemoveChild(ContentPane);

            FrameObject = nullptr;
        }
        ContentPane = Obj;
        if (ContentPane != nullptr)
        {
            AddChild(ContentPane);
            SetSize(ContentPane->GetSize());
            ContentPane->AddRelation(this, ERelationType::Size);
            FrameObject = Cast<UGComponent>(ContentPane->GetChild("frame"));
            if (FrameObject != nullptr)
            {
                SetCloseButton(FrameObject->GetChild("closeButton"));
                SetDragArea(FrameObject->GetChild("dragArea"));
                SetContentArea(FrameObject->GetChild("contentArea"));
            }
        }
        else
            FrameObject = nullptr;
    }
}

void UGWindow::SetCloseButton(UGObject * Obj)
{
    if (CloseButton != Obj)
    {
        if (CloseButton != nullptr)
            CloseButton->OnClick.RemoveDynamic(this, &UGWindow::CloseEventHandler);
        CloseButton = Obj;
        if (CloseButton != nullptr)
            CloseButton->OnClick.AddUniqueDynamic(this, &UGWindow::CloseEventHandler);
    }
}

void UGWindow::SetDragArea(UGObject * Obj)
{
    if (DragArea != Obj)
    {
        if (DragArea != nullptr)
        {
            DragArea->SetDraggable(false);
            DragArea->OnDragStart.RemoveDynamic(this, &UGWindow::OnDragStartHandler);
        }

        DragArea = Obj;
        if (DragArea != nullptr)
        {
            UGGraph* DragGraph;
            if ((DragGraph = Cast<UGGraph>(DragArea)) != nullptr && DragGraph->IsEmpty())
                DragGraph->DrawRect(0, FColor::Transparent, FColor::Transparent);

            DragArea->SetDraggable(true);
            DragArea->OnDragStart.AddUniqueDynamic(this, &UGWindow::OnDragStartHandler);
        }
    }
}

void UGWindow::Show()
{
    GetUIRoot()->ShowWindow(this);
}

void UGWindow::Hide()
{
    if (IsShowing())
        DoHideAnimation();
}

void UGWindow::HideImmediately()
{
    GetUIRoot()->HideWindowImmediately(this);
}

void UGWindow::ToggleStatus()
{
    if (IsTop())
        Hide();
    else
        Show();
}

void UGWindow::BringToFront()
{
    GetUIRoot()->BringToFront(this);
}

bool UGWindow::IsTop() const
{
    return Parent.IsValid() && Parent->GetChildIndex(this) == Parent->NumChildren() - 1;
}

void UGWindow::ShowModalWait(int32 InRequestingCmd)
{
    if (InRequestingCmd != 0)
        RequestingCmd = InRequestingCmd;

    if (!FUIConfig::Config.WindowModalWaiting.IsEmpty())
    {
        if (ModalWaitPane == nullptr)
            ModalWaitPane = UUIPackage::CreateObjectFromURL(FUIConfig::Config.WindowModalWaiting, this);

        LayoutModalWaitPane();

        AddChild(ModalWaitPane);
    }
}

void UGWindow::LayoutModalWaitPane()
{
    if (ContentArea != nullptr)
    {
        FVector2D pt = FrameObject->LocalToGlobal(FVector2D::ZeroVector);
        pt = GlobalToLocal(pt);
        ModalWaitPane->SetPosition(pt + ContentArea->GetPosition());
        ModalWaitPane->SetSize(ContentArea->GetSize());
    }
    else
        ModalWaitPane->SetSize(Size);
}

bool UGWindow::CloseModalWait(int32 InRequestingCmd)
{
    if (InRequestingCmd != 0)
    {
        if (RequestingCmd != InRequestingCmd)
            return false;
    }
    RequestingCmd = 0;

    if (ModalWaitPane != nullptr && ModalWaitPane->GetParent() != nullptr)
        RemoveChild(ModalWaitPane);

    return true;
}

void UGWindow::Init()
{
    if (bInited || bLoading)
        return;

    if (UISources.Num() > 0)
    {
        bLoading = false;
        int32 cnt = UISources.Num();
        for (int32 i = 0; i < cnt; i++)
        {
            const TSharedPtr<IUISource>& lib = UISources[i];
            if (!lib->IsLoaded())
            {
                lib->Load(FSimpleDelegate::CreateUObject(this, &UGWindow::OnUILoadComplete));
                bLoading = true;
            }
        }

        if (!bLoading)
            InternalInit();
    }
    else
        InternalInit();
}

void UGWindow::InternalInit()
{
    bInited = true;
    OnInit();

    if (IsShowing())
        DoShowAnimation();
}

void UGWindow::AddUISource(TSharedPtr<IUISource> UISource)
{
    UISources.Add(UISource);
}

void UGWindow::OnInit()
{
    InitCallback.ExecuteIfBound(this);
}

void UGWindow::OnShown()
{
    ShownCallback.ExecuteIfBound(this);
}

void UGWindow::OnHide()
{
    HideCallback.ExecuteIfBound(this);
}

void UGWindow::DoShowAnimation()
{
    if (ShowingCallback.IsBound())
        ShowingCallback.Execute(this);
    else
        OnShown();
}

void UGWindow::DoHideAnimation()
{
    if (HidingCallback.IsBound())
        HidingCallback.Execute(this);
    else
        HideImmediately();
}

void UGWindow::CloseEventHandler(UEventContext * Context)
{
    Hide();
}

void UGWindow::OnUILoadComplete()
{
    int32 cnt = UISources.Num();
    for (int32 i = 0; i < cnt; i++)
    {
        const TSharedPtr<IUISource>& lib = UISources[i];
        if (!lib->IsLoaded())
            return;
    }

    bLoading = false;
    InternalInit();
}

void UGWindow::OnAddedToStageHandler(UEventContext * Context)
{
    if (!bInited)
        Init();
    else
        DoShowAnimation();
}

void UGWindow::OnRemovedFromStageHandler(UEventContext * Context)
{
    CloseModalWait();
    OnHide();
}

void UGWindow::OnTouchBeginHandler(UEventContext * Context)
{
    if (IsShowing() && bBringToFontOnClick)
    {
        BringToFront();
    }
}

void UGWindow::OnDragStartHandler(UEventContext * Context)
{
    Context->PreventDefault();

    StartDrag(Context->GetUserIndex(), Context->GetPointerIndex());
}