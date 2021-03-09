#include "FairyApplication.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SGameLayerManager.h"
#include "UI/GRoot.h"
#include "UI/UIPackage.h"
#include "UI/UIObjectFactory.h"
#include "Tween/TweenManager.h"
#include "Widgets/NTexture.h"
#if WITH_EDITOR
#include "Editor.h"
#endif

UFairyApplication* UFairyApplication::Instance = nullptr;

#if WITH_EDITOR
//don t know why FScopedSwitchWorldHack not work!
struct FMyScopedSwitchWorldHack
{
    FMyScopedSwitchWorldHack(const TSharedPtr<SWindow>& InWindow)
        : PrevWorld(nullptr)
    {
        if (GIsEditor)
        {
			UWorld* PIEWorld = GEditor->GetPIEWorldContext()->World();
			if (GWorld != PIEWorld)
			{
				PrevWorld = GWorld;
				GWorld = PIEWorld;
			}
			else {
				PrevWorld = nullptr;
			}
        }
    }

    ~FMyScopedSwitchWorldHack()
    {
        if (GIsEditor)
        {
			if (PrevWorld != nullptr)
			{
				GWorld = PrevWorld;
			}
        }
    }
    UWorld* PrevWorld;
};
#else
struct FMyScopedSwitchWorldHack
{
    FMyScopedSwitchWorldHack(const TSharedPtr<SWindow>& InWindow) {}
};
#endif

UFairyApplication::FTouchInfo::FTouchInfo() :
    UserIndex(0),
    PointerIndex(0),
    bDown(false),
    DownPosition(0, 0),
    bClickCancelled(false),
    ClickCount(0)
{
}

void UFairyApplication::FInputProcessor::Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor)
{
}

bool UFairyApplication::FInputProcessor::HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
    if (!UFairyApplication::IsStarted())
        return false;

    UFairyApplication::Get()->PreviewDownEvent(MouseEvent);

    return false;
}

bool UFairyApplication::FInputProcessor::HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
    if (!UFairyApplication::IsStarted())
        return false;

    UFairyApplication::Get()->PreviewUpEvent(MouseEvent);
    return false;
}

bool UFairyApplication::FInputProcessor::HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
    if (!UFairyApplication::IsStarted())
        return false;

    UFairyApplication::Get()->PreviewMoveEvent(MouseEvent);
    return false;
}

UFairyApplication* UFairyApplication::Get()
{
    if (Instance != nullptr)
        return Instance;

    Instance = NewObject<UFairyApplication>();
    Instance->AddToRoot();
    Instance->OnCreate();
    return Instance;
}

void UFairyApplication::Destroy()
{
    if (Instance != nullptr)
    {
        Instance->RemoveFromRoot();
        Instance->OnDestroy();
    }
    Instance = nullptr;
}

UFairyApplication::UFairyApplication() :
    bSoundEnabled(true),
    SoundVolumeScale(1)
{
    LastTouch = new FTouchInfo();
    Touches.Add(LastTouch);
}

void UFairyApplication::OnCreate()
{
    InputProcessor = MakeShareable(new FInputProcessor());
    FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor);

    ViewportClient = GWorld->GetGameViewport();
    ViewportWidget = Instance->ViewportClient->GetGameViewportWidget();

    DragDropManager = NewObject<UDragDropManager>();

    PostTickDelegateHandle = FSlateApplication::Get().OnPostTick().AddUObject(this, &UFairyApplication::OnSlatePostTick);
}

void UFairyApplication::OnDestroy()
{
    UUIPackage::RemoveAllPackages();
    UUIPackage::Branch.Reset();
    UUIPackage::Vars.Reset();

    FUIObjectFactory::PackageItemExtensions.Reset();
    FUIObjectFactory::LoaderCreator.Unbind();

    UGRoot::Instance = nullptr;
    UNTexture::DestroyWhiteTexture();
    FUIConfig::Config = FUIConfig(); //Reset Configuration to default values

    if (InputProcessor.IsValid())
        FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor);

    if(PostTickDelegateHandle.IsValid())
        FSlateApplication::Get().OnPostTick().Remove(PostTickDelegateHandle);
}

void UFairyApplication::CallAfterSlateTick(FSimpleDelegate Callback)
{
    PostTickMulticastDelegate.Add(Callback);
}

void UFairyApplication::OnSlatePostTick(float DeltaTime)
{
    if (PostTickMulticastDelegate.IsBound())
    {
        FSimpleMulticastDelegate Clone = PostTickMulticastDelegate;
        PostTickMulticastDelegate.Clear();
        Clone.Broadcast();
    }

    if (bNeedCheckPopups)
    {
        bNeedCheckPopups = false;

        if (UIRoot != nullptr)
            UIRoot->CheckPopups(nullptr);
    }
}

FVector2D UFairyApplication::GetTouchPosition(int32 InUserIndex, int32 InPointerIndex)
{
    FTouchInfo* TouchInfo = GetTouchInfo(InUserIndex, InPointerIndex);
    if (TouchInfo != nullptr)
        return TouchInfo->Event.GetScreenSpacePosition();
    else
        return FVector2D::ZeroVector;
}

int32 UFairyApplication::GetTouchCount() const
{
    int32 Count = 0;
    for (auto &it : Touches)
    {
        if (it.bDown)
            Count++;
    }

    return Count;
}

UGObject* UFairyApplication::GetObjectUnderPoint(const FVector2D& ScreenspacePosition)
{
    TArray<TSharedRef<SWindow>> Windows;
    Windows.Add(ViewportClient->GetWindow().ToSharedRef());
    FWidgetPath WidgetPath = FSlateApplication::Get().LocateWindowUnderMouse(ScreenspacePosition, Windows, false);

    if (WidgetPath.IsValid())
        return GetWidgetGObject(WidgetPath.GetLastWidget());
    else
        return nullptr;
}

void UFairyApplication::CancelClick(int32 InUserIndex, int32 InPointerIndex)
{
    FTouchInfo* TouchInfo = GetTouchInfo(InUserIndex, InPointerIndex);
    if (TouchInfo != nullptr)
        TouchInfo->bClickCancelled = true;
}

void UFairyApplication::PlaySound(const FString& URL, float VolumnScale)
{
    if (!bSoundEnabled)
        return;

    TSharedPtr<FPackageItem> SoundItem = UUIPackage::GetItemByURL(URL);
    if (SoundItem.IsValid())
    {
        SoundItem->Load();
        FSlateApplication::Get().PlaySound(*SoundItem->Sound);
    }
}

void UFairyApplication::SetSoundEnabled(bool bEnabled)
{
    bSoundEnabled = bEnabled;
}

void UFairyApplication::SetSoundVolumeScale(float VolumnScale)
{
    SoundVolumeScale = VolumnScale;
}

bool UFairyApplication::DispatchEvent(const FName& EventType, const TSharedRef<SWidget>& Initiator, const FNVariant& Data)
{
    UGObject* Obj = GetWidgetGObject(Initiator);
    if (Obj == nullptr)
        return false;

    UEventContext* Context = BorrowEventContext();
    Context->Type = EventType;
    Context->Initiator = Obj;
    Context->Sender = Obj;
    Context->Data = Data;

    Context->Sender->InvokeEventDelegate(Context);

    ReturnEventContext(Context);

    return Context->bDefaultPrevented;
}

void UFairyApplication::BubbleEvent(const FName& EventType, const TSharedRef<SWidget>& Initiator, const FNVariant& Data)
{
    TArray<UGObject*> CallChain;
    GetPathToRoot(Initiator, CallChain);
    if (CallChain.Num() == 0)
        return;

    InternalBubbleEvent(EventType, CallChain, Data);
}

void UFairyApplication::InternalBubbleEvent(const FName& EventType, const TArray<UGObject*>& CallChain, const FNVariant& Data)
{
    UEventContext* Context = BorrowEventContext();
    Context->Type = EventType;
    Context->Initiator = CallChain[0];
    Context->Data = Data;

    for (auto& it : CallChain)
    {
        Context->Sender = it;
        it->InvokeEventDelegate(Context);

        if (Context->bIsMouseCaptor)
        {
            Context->bIsMouseCaptor = false;
            AddMouseCaptor(Context->GetUserIndex(), (int32)Context->GetPointerIndex(), it);
        }

        if (Context->IsPropagationStopped())
            break;
    }

    ReturnEventContext(Context);
}

void UFairyApplication::BroadcastEvent(const FName& EventType, const TSharedRef<SWidget>& Initiator, const FNVariant& Data)
{
    TArray<UGObject*> CallChain;
    GetDescendants(Initiator, CallChain);
    if (CallChain.Num() == 0)
        return;

    UEventContext* Context = BorrowEventContext();
    Context->Type = EventType;
    Context->Data = Data;

    for (auto& it : CallChain)
    {
        Context->Sender = it;
        Context->Initiator = it;
        it->InvokeEventDelegate(Context);
    }

    ReturnEventContext(Context);
}

UGObject* UFairyApplication::GetWidgetGObject(const TSharedPtr<SWidget>& InWidget)
{
    if (!InWidget.IsValid())
        return nullptr;

    TSharedPtr<SWidget> Ptr = InWidget;
    while (Ptr.IsValid() && Ptr != ViewportWidget)
    {
        if (Ptr->GetTag() == SDisplayObject::SDisplayObjectTag)
        {
            const TWeakObjectPtr<UGObject>& ObjPtr = StaticCastSharedPtr<SDisplayObject>(Ptr)->GObject;
            if (ObjPtr.IsValid())
                return ObjPtr.Get();
        }

        Ptr = Ptr->GetParentWidget();
    }

    return nullptr;
}

void UFairyApplication::GetPathToRoot(const TSharedRef<SWidget>& InWidget, TArray<UGObject*>& OutArray)
{
    TSharedPtr<SWidget> Ptr = InWidget;
    while (Ptr.IsValid() && Ptr != ViewportWidget)
    {
        if (Ptr->GetTag() == SDisplayObject::SDisplayObjectTag)
        {
            const TWeakObjectPtr<UGObject>& ObjPtr = StaticCastSharedPtr<SDisplayObject>(Ptr)->GObject;
            if (ObjPtr.IsValid())
                OutArray.Add(ObjPtr.Get());
        }

        Ptr = Ptr->GetParentWidget();
    }
}

void UFairyApplication::GetDescendants(const TSharedRef<SWidget>& InWidget, TArray<UGObject*>& OutArray)
{
    if (InWidget->GetTag() == SDisplayObject::SDisplayObjectTag)
    {
        const TSharedRef<SDisplayObject>& DisplayObject = StaticCastSharedRef<SDisplayObject>(InWidget);
        if (DisplayObject->GObject.IsValid())
            OutArray.Add(DisplayObject->GObject.Get());
    }

    FChildren* Children = InWidget->GetChildren();
    for (int32 SlotIdx = 0; SlotIdx < Children->Num(); ++SlotIdx)
    {
        GetDescendants(Children->GetChildAt(SlotIdx), OutArray);
    }
}

UEventContext* UFairyApplication::BorrowEventContext()
{
    UEventContext* Context;
    if (EventContextPool.Num() > 0)
    {
        Context = EventContextPool.Pop();
        Context->bDefaultPrevented = false;
        Context->bStopped = false;
        Context->bIsMouseCaptor = false;
    }
    else
        Context = NewObject<UEventContext>(this);
    Context->PointerEvent = &LastTouch->Event;
    //Context->KeyEvent = &LastKeyEvent;

    return Context;
}

void UFairyApplication::ReturnEventContext(UEventContext* Context)
{
    Context->Data.Reset();
    EventContextPool.Add(Context);
}

void UFairyApplication::AddMouseCaptor(int32 InUserIndex, int32 InPointerIndex, UGObject* InTarget)
{
    FTouchInfo* TouchInfo = GetTouchInfo(InUserIndex, InPointerIndex);
    if (TouchInfo != nullptr && !TouchInfo->MouseCaptors.Contains(InTarget))
        TouchInfo->MouseCaptors.Add(InTarget);
}

void UFairyApplication::RemoveMouseCaptor(int32 InUserIndex, int32 InPointerIndex, UGObject* InTarget)
{
    FTouchInfo* TouchInfo = GetTouchInfo(InUserIndex, InPointerIndex);
    if (TouchInfo != nullptr)
        TouchInfo->MouseCaptors.Remove(InTarget);
}

bool UFairyApplication::HasMouseCaptor(int32 InUserIndex, int32 InPointerIndex)
{
    FTouchInfo* TouchInfo = GetTouchInfo(InUserIndex, InPointerIndex);
    return TouchInfo != nullptr && TouchInfo->MouseCaptors.Num() > 0;
}

UFairyApplication::FTouchInfo* UFairyApplication::GetTouchInfo(const FPointerEvent& MouseEvent)
{
    FTouchInfo* TouchInfo = nullptr;
    for (auto &it : Touches)
    {
        if (it.UserIndex == MouseEvent.GetUserIndex() && it.PointerIndex == (int32)MouseEvent.GetPointerIndex())
        {
            TouchInfo = &it;
            break;
        }
    }

    if (TouchInfo == nullptr)
    {
        TouchInfo = new FTouchInfo();
        Touches.Add(TouchInfo);
        TouchInfo->UserIndex = MouseEvent.GetUserIndex();
        TouchInfo->PointerIndex = (int32)MouseEvent.GetPointerIndex();
    }

    LastTouch = TouchInfo;
    return TouchInfo;
}

UFairyApplication::FTouchInfo* UFairyApplication::GetTouchInfo(int32 InUserIndex, int32 InPointerIndex)
{
    if (InUserIndex == -1 && InPointerIndex == -1)
        return LastTouch;

    for (auto &it : Touches)
    {
        if (it.UserIndex == InUserIndex && it.PointerIndex == InPointerIndex)
        {
            return &it;
        }
    }
    return nullptr;
}

void UFairyApplication::PreviewDownEvent(const FPointerEvent& MouseEvent)
{
    FTouchInfo* TouchInfo = GetTouchInfo(MouseEvent);
    TouchInfo->Event = MouseEvent;
    TouchInfo->bDown = true;
    TouchInfo->DownPosition = MouseEvent.GetScreenSpacePosition();
    TouchInfo->bClickCancelled = false;
    TouchInfo->ClickCount = 1;
    TouchInfo->MouseCaptors.Reset();
    TouchInfo->bToClearCaptors = false;
    TouchInfo->DownPath.Reset();

    bNeedCheckPopups = true;
}

void UFairyApplication::PreviewUpEvent(const FPointerEvent& MouseEvent)
{
    FTouchInfo* TouchInfo = GetTouchInfo(MouseEvent);
    TouchInfo->Event = MouseEvent;
    TouchInfo->bDown = false;
    TouchInfo->bToClearCaptors = true;

    if (TouchInfo->MouseCaptors.Num() > 0)
    {
        FMyScopedSwitchWorldHack SwitchWorld(ViewportClient->GetWindow());

        int32 cnt = TouchInfo->MouseCaptors.Num();
        for (int32 i = 0; i < cnt; i++)
        {
            auto& Captor = TouchInfo->MouseCaptors[i];
            if (Captor.IsValid() && Captor->OnStage())
            {
                DispatchEvent(FUIEvents::TouchEnd, Captor->GetDisplayObject());
            }
        }
    }
}

void UFairyApplication::PreviewMoveEvent(const FPointerEvent& MouseEvent)
{
    FTouchInfo* TouchInfo = GetTouchInfo(MouseEvent);
    TouchInfo->Event = MouseEvent;

    if ((TouchInfo->DownPosition - MouseEvent.GetScreenSpacePosition()).GetAbsMax() > 50)
        TouchInfo->bClickCancelled = true;

    if (!TouchInfo->bToClearCaptors && TouchInfo->MouseCaptors.Num() > 0)
    {
        FMyScopedSwitchWorldHack SwitchWorld(ViewportClient->GetWindow());

        int32 cnt = TouchInfo->MouseCaptors.Num();
        for (int32 i = 0; i < cnt; i++)
        {
            auto& Captor = TouchInfo->MouseCaptors[i];
            if (Captor.IsValid() && Captor->OnStage())
            {
                DispatchEvent(FUIEvents::TouchMove, Captor->GetDisplayObject());
            }
        }
    }
}

FReply UFairyApplication::OnWidgetMouseButtonDown(const TSharedRef<SWidget>& Widget, const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    FMyScopedSwitchWorldHack SwitchWorld(ViewportClient->GetWindow());

    FTouchInfo* TouchInfo = GetTouchInfo(MouseEvent);

    UGObject* InitialGObject = nullptr;
    TSharedPtr<SWidget> Ptr = Widget;
    while (Ptr.IsValid() && Ptr != ViewportWidget)
    {
        TouchInfo->DownPath.Add(Ptr);

        if (InitialGObject == nullptr && Ptr->GetTag() == SDisplayObject::SDisplayObjectTag)
        {
            InitialGObject = StaticCastSharedPtr<SDisplayObject>(Ptr)->GObject.Get();
        }

        Ptr = Ptr->GetParentWidget();
    }

    bNeedCheckPopups = false;
    if (UIRoot != nullptr)
        UIRoot->CheckPopups(&Widget.Get());

    BubbleEvent(FUIEvents::TouchBegin, Widget);

    return FReply::Handled();
}

FReply UFairyApplication::OnWidgetMouseButtonUp(const TSharedRef<SWidget>& Widget, const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    FMyScopedSwitchWorldHack SwitchWorld(ViewportClient->GetWindow());

    FTouchInfo* TouchInfo = GetTouchInfo(MouseEvent);
    if (TouchInfo == nullptr)
        return FReply::Handled().ReleaseMouseCapture();

    TArray<UGObject*> CallChain;
    GetPathToRoot(Widget, CallChain);
    if (CallChain.Num() > 0)
    {
        for (auto& it : TouchInfo->MouseCaptors)
        {
            if (it.IsValid())
                CallChain.RemoveSingle(it.Get());
        }

        if (CallChain.Num() > 0)
            InternalBubbleEvent(FUIEvents::TouchEnd, CallChain, FNVariant::Null);
    }
    TouchInfo->MouseCaptors.Reset();

    if (!TouchInfo->bClickCancelled)
    {
        TSharedPtr<SWidget> Ptr = Widget;
        while (Ptr.IsValid() && Ptr != ViewportWidget)
        {
            if (TouchInfo->DownPath.Contains(Ptr))
            {
                BubbleEvent(FUIEvents::Click, Ptr.ToSharedRef());
                break;
            }

            Ptr = Ptr->GetParentWidget();
        }
    }

    TouchInfo->DownPath.Reset();

    return FReply::Handled().ReleaseMouseCapture();
}

FReply UFairyApplication::OnWidgetMouseMove(const TSharedRef<SWidget>& Widget, const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    return FReply::Handled();
}

FReply UFairyApplication::OnWidgetMouseButtonDoubleClick(const TSharedRef<SWidget>& Widget, const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    return OnWidgetMouseButtonDown(Widget, MyGeometry, MouseEvent);
}

void UFairyApplication::OnWidgetMouseEnter(const TSharedRef<SWidget>& Widget, const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    FMyScopedSwitchWorldHack SwitchWorld(ViewportClient->GetWindow());

    FTouchInfo* TouchInfo = GetTouchInfo(MouseEvent);
    DispatchEvent(FUIEvents::RollOver, Widget);
}

void UFairyApplication::OnWidgetMouseLeave(const TSharedRef<SWidget>& Widget, const FPointerEvent& MouseEvent)
{
    FMyScopedSwitchWorldHack SwitchWorld(ViewportClient->GetWindow());

    FTouchInfo* TouchInfo = GetTouchInfo(MouseEvent);
    DispatchEvent(FUIEvents::RollOut, Widget);
}

FReply UFairyApplication::OnWidgetMouseWheel(const TSharedRef<SWidget>& Widget, const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    FMyScopedSwitchWorldHack SwitchWorld(ViewportClient->GetWindow());

    FTouchInfo* TouchInfo = GetTouchInfo(MouseEvent);
    TouchInfo->Event = MouseEvent;

    BubbleEvent(FUIEvents::MouseWheel, Widget);

    return FReply::Handled();
}