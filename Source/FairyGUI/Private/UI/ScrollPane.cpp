#include "UI/ScrollPane.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "TimerManager.h"
#include "UI/UIPackage.h"
#include "UI/GList.h"
#include "UI/GController.h"
#include "UI/GScrollBar.h"
#include "Utils/ByteBuffer.h"
#include "Tween/GTween.h"
#include "Widgets/SContainer.h"
#include "FairyApplication.h"

TWeakObjectPtr<UScrollPane> UScrollPane::DraggingPane;
int32 UScrollPane::GestureFlag = 0;

static const float TWEEN_TIME_GO = 0.5f;      //tween time for SetPos(ani)
static const float TWEEN_TIME_DEFAULT = 0.3f; //min tween time for inertial scroll
static const float PULL_RATIO = 0.5f;         //pull down/up ratio

static inline float sp_EaseFunc(float t, float d)
{
    t = t / d - 1;
    return t * t * t + 1; //cubicOut
}

UScrollPane::UScrollPane()
{
}

UScrollPane::~UScrollPane()
{
}

void UScrollPane::Setup(FByteBuffer* Buffer)
{
    Owner = Cast<UGComponent>(GetOuter());
    Container = Owner->Container;
    ScrollStep = FUIConfig::Config.DefaultScrollStep;
    DecelerationRate = FUIConfig::Config.DefaultScrollDecelerationRate;
    bTouchEffect = FUIConfig::Config.DefaultScrollTouchEffect;
    bBouncebackEffect = FUIConfig::Config.DefaultScrollBounceEffect;
    bMouseWheelEnabled = true;
    PageSize.Set(0, 0);

    MaskContainer = SNew(SContainer);
    MaskContainer->SetOpaque(false);
    Owner->RootContainer->RemoveChild(Container.ToSharedRef());
    Owner->RootContainer->AddChild(MaskContainer.ToSharedRef());
    MaskContainer->AddChild(Container.ToSharedRef());

    Owner->On(FUIEvents::MouseWheel).AddUObject(this, &UScrollPane::OnMouseWheel);
    Owner->On(FUIEvents::TouchBegin).AddUObject(this, &UScrollPane::OnTouchBegin);
    Owner->On(FUIEvents::TouchMove).AddUObject(this, &UScrollPane::OnTouchMove);
    Owner->On(FUIEvents::TouchEnd).AddUObject(this, &UScrollPane::OnTouchEnd);
    Owner->On(FUIEvents::RemovedFromStage).AddLambda([this](UEventContext*) {
        if (DraggingPane.Get() == this)
            DraggingPane.Reset();
    });

    ScrollType = (EScrollType)Buffer->ReadByte();
    EScrollBarDisplayType scrollBarDisplay = (EScrollBarDisplayType)Buffer->ReadByte();
    int32 flags = Buffer->ReadInt();

    if (Buffer->ReadBool())
    {
        ScrollBarMargin.Top = Buffer->ReadInt();
        ScrollBarMargin.Bottom = Buffer->ReadInt();
        ScrollBarMargin.Left = Buffer->ReadInt();
        ScrollBarMargin.Right = Buffer->ReadInt();
    }

    const FString& vtScrollBarRes = Buffer->ReadS();
    const FString& hzScrollBarRes = Buffer->ReadS();
    const FString& headerRes = Buffer->ReadS();
    const FString& footerRes = Buffer->ReadS();

    bDisplayOnLeft = (flags & 1) != 0;
    bSnapToItem = (flags & 2) != 0;
    bDisplayInDemand = (flags & 4) != 0;
    bPageMode = (flags & 8) != 0;
    if ((flags & 16) != 0)
        bTouchEffect = true;
    else if ((flags & 32) != 0)
        bTouchEffect = false;
    if ((flags & 64) != 0)
        bBouncebackEffect = true;
    else if ((flags & 128) != 0)
        bBouncebackEffect = false;
    bInertiaDisabled = (flags & 256) != 0;
    if ((flags & 512) == 0)
        MaskContainer->SetClipping(EWidgetClipping::ClipToBoundsAlways);
    bFloating = (flags & 1024) != 0;
    bDontClipMargin = (flags & 2048) != 0;

    if (scrollBarDisplay == EScrollBarDisplayType::Default)
        scrollBarDisplay = FUIConfig::Config.DefaultScrollBarDisplay;

    if (scrollBarDisplay != EScrollBarDisplayType::Hidden)
    {
        if (ScrollType == EScrollType::Both || ScrollType == EScrollType::Vertical)
        {
            
            const FString& res = vtScrollBarRes.Len() == 0 ? FUIConfig::Config.VerticalScrollBar : vtScrollBarRes;
            if (res.Len() > 0)
            {
                VtScrollBar = Cast<UGScrollBar>(UUIPackage::CreateObjectFromURL(res, Owner));
                if (VtScrollBar == nullptr)
                {
                    UE_LOG(LogFairyGUI, Warning, TEXT("cannot create scrollbar from %s"), *res);
                }
                else
                {
                    VtScrollBar->SetScrollPane(this, true);
                    Owner->RootContainer->AddChild(VtScrollBar->GetDisplayObject());
                }
            }
        }
        if (ScrollType == EScrollType::Both || ScrollType == EScrollType::Horizontal)
        {
            const FString& res = hzScrollBarRes.Len() == 0 ? FUIConfig::Config.HorizontalScrollBar : hzScrollBarRes;
            if (res.Len() > 0)
            {
                HzScrollBar = Cast<UGScrollBar>(UUIPackage::CreateObjectFromURL(res, Owner));
                if (HzScrollBar == nullptr)
                {
                    UE_LOG(LogFairyGUI, Warning, TEXT("cannot create scrollbar from %s"), *res);
                }
                else
                {
                    HzScrollBar->SetScrollPane(this, false);
                    Owner->RootContainer->AddChild(HzScrollBar->GetDisplayObject());
                }
            }
        }

        bScrollBarDisplayAuto = scrollBarDisplay == EScrollBarDisplayType::Auto;
        if (bScrollBarDisplayAuto)
        {
            if (VtScrollBar != nullptr)
                VtScrollBar->SetVisible(false);
            if (HzScrollBar != nullptr)
                HzScrollBar->SetVisible(false);

            Owner->On(FUIEvents::RollOver).AddUObject(this, &UScrollPane::OnRollOver);
            Owner->On(FUIEvents::RollOut).AddUObject(this, &UScrollPane::OnRollOut);
        }
    }
    else
        bMouseWheelEnabled = false;

    if (headerRes.Len() > 0)
    {
        Header = Cast<UGComponent>(UUIPackage::CreateObjectFromURL(headerRes, Owner));
        if (Header == nullptr)
        {
            UE_LOG(LogFairyGUI, Warning, TEXT("cannot create UScrollPane header from %s"), *headerRes);
        }
        else
        {
            Header->SetVisible(false);
            Owner->RootContainer->AddChild(Header->GetDisplayObject());
        }
    }

    if (footerRes.Len() > 0)
    {
        Footer = Cast<UGComponent>(UUIPackage::CreateObjectFromURL(footerRes, Owner));
        if (Footer == nullptr)
        {
            UE_LOG(LogFairyGUI, Warning, TEXT("cannot create UScrollPane footer from %s"), *footerRes);
        }
        else
        {
            Footer->SetVisible(false);
            Owner->RootContainer->AddChild(Footer->GetDisplayObject());
        }
    }

    if (Header != nullptr || Footer != nullptr)
        RefreshBarAxis = (ScrollType == EScrollType::Both || ScrollType == EScrollType::Vertical) ? 1 : 0;

    SetSize(Owner->GetSize());
}

void UScrollPane::SetPosX(float Value, bool bAnimation)
{
    Owner->EnsureBoundsCorrect();

    if (LoopMode == 1)
        LoopCheckingNewPos(Value, 0);

    Value = FMath::Clamp(Value, 0.f, OverlapSize.X);
    if (Value != XPos)
    {
        XPos = Value;
        PosChanged(bAnimation);
    }
}

void UScrollPane::SetPosY(float Value, bool bAnimation)
{
    Owner->EnsureBoundsCorrect();

    if (LoopMode == 2)
        LoopCheckingNewPos(Value, 1);

    Value = FMath::Clamp(Value, 0.f, OverlapSize.Y);
    if (Value != YPos)
    {
        YPos = Value;
        PosChanged(bAnimation);
    }
}

float UScrollPane::GetPercX() const
{
    return OverlapSize.X == 0 ? 0 : XPos / OverlapSize.X;
}

void UScrollPane::SetPercX(float Value, bool bAnimation)
{
    Owner->EnsureBoundsCorrect();
    SetPosX(OverlapSize.X * FMath::Clamp(Value, 0.f, 1.f), bAnimation);
}

float UScrollPane::GetPercY() const
{
    return OverlapSize.Y == 0 ? 0 : YPos / OverlapSize.Y;
}

void UScrollPane::SetPercY(float Value, bool bAnimation)
{
    Owner->EnsureBoundsCorrect();
    SetPosY(OverlapSize.Y * FMath::Clamp(Value, 0.f, 1.f), bAnimation);
}

bool UScrollPane::IsBottomMost() const
{
    return YPos == OverlapSize.Y || OverlapSize.Y == 0;
}

bool UScrollPane::IsRightMost() const
{
    return XPos == OverlapSize.X || OverlapSize.X == 0;
}

void UScrollPane::ScrollLeft(float Ratio, bool bAnimation)
{
    if (bPageMode)
        SetPosX(XPos - PageSize.X * Ratio, bAnimation);
    else
        SetPosX(XPos - ScrollStep * Ratio, bAnimation);
}

void UScrollPane::ScrollRight(float Ratio, bool bAnimation)
{
    if (bPageMode)
        SetPosX(XPos + PageSize.X * Ratio, bAnimation);
    else
        SetPosX(XPos + ScrollStep * Ratio, bAnimation);
}

void UScrollPane::ScrollUp(float Ratio, bool bAnimation)
{
    if (bPageMode)
        SetPosY(YPos - PageSize.Y * Ratio, bAnimation);
    else
        SetPosY(YPos - ScrollStep * Ratio, bAnimation);
}

void UScrollPane::ScrollDown(float Ratio, bool bAnimation)
{
    if (bPageMode)
        SetPosY(YPos + PageSize.Y * Ratio, bAnimation);
    else
        SetPosY(YPos + ScrollStep * Ratio, bAnimation);
}

void UScrollPane::ScrollTop(bool bAnimation)
{
    SetPercY(0, bAnimation);
}

void UScrollPane::ScrollBottom(bool bAnimation)
{
    SetPercY(1, bAnimation);
}

void UScrollPane::ScrollToView(UGObject* Obj, bool bAnimation, bool bSetFirst)
{
    Owner->EnsureBoundsCorrect();
    if (bNeedRefresh)
        Refresh();

    FBox2D rect(Obj->GetPosition(), Obj->GetPosition() + Obj->GetSize());
    if (Obj->GetParent() != Owner)
    {
        rect = Obj->GetParent()->LocalToGlobalRect(rect);
        rect = Owner->GlobalToLocalRect(rect);
    }
    ScrollToView(rect, bAnimation, bSetFirst);
}

void UScrollPane::ScrollToView(const FBox2D& Rect, bool bAnimation, bool bSetFirst)
{
    Owner->EnsureBoundsCorrect();
    if (bNeedRefresh)
        Refresh();

    if (OverlapSize.Y > 0)
    {
        float bottom = YPos + ViewSize.Y;
        if (bSetFirst || Rect.Min.Y <= YPos || Rect.GetSize().Y >= ViewSize.Y)
        {
            if (bPageMode)
                SetPosY(FMath::FloorToFloat(Rect.Min.Y / PageSize.Y) * PageSize.Y, bAnimation);
            else
                SetPosY(Rect.Min.Y, bAnimation);
        }
        else if (Rect.Max.Y > bottom)
        {
            if (bPageMode)
                SetPosY(FMath::FloorToFloat(Rect.Min.Y / PageSize.Y) * PageSize.Y, bAnimation);
            else if (Rect.GetSize().Y <= ViewSize.Y / 2)
                SetPosY(Rect.Min.Y + Rect.GetSize().Y * 2 - ViewSize.Y, bAnimation);
            else
                SetPosY(Rect.Max.Y - ViewSize.Y, bAnimation);
        }
    }
    if (OverlapSize.X > 0)
    {
        float right = XPos + ViewSize.X;
        if (bSetFirst || Rect.Min.X <= XPos || Rect.GetSize().X >= ViewSize.X)
        {
            if (bPageMode)
                SetPosX(FMath::FloorToFloat(Rect.Min.X / PageSize.X) * PageSize.X, bAnimation);
            SetPosX(Rect.Min.X, bAnimation);
        }
        else if (Rect.Max.X > right)
        {
            if (bPageMode)
                SetPosX(FMath::FloorToFloat(Rect.Min.X / PageSize.X) * PageSize.X, bAnimation);
            else if (Rect.GetSize().X <= ViewSize.X / 2)
                SetPosX(Rect.Min.X + Rect.GetSize().X * 2 - ViewSize.X, bAnimation);
            else
                SetPosX(Rect.Max.X - ViewSize.X, bAnimation);
        }
    }

    if (!bAnimation && bNeedRefresh)
        Refresh();
}

bool UScrollPane::IsChildInView(UGObject* Obj) const
{
    if (OverlapSize.Y > 0)
    {
        float dist = Obj->GetY() + Container->GetPosition().Y;
        if (dist <= -Obj->GetHeight() || dist >= ViewSize.Y)
            return false;
    }
    if (OverlapSize.X > 0)
    {
        float dist = Obj->GetX() + Container->GetPosition().X;
        if (dist <= -Obj->GetWidth() || dist >= ViewSize.X)
            return false;
    }

    return true;
}

int32 UScrollPane::GetPageX() const
{
    if (!bPageMode)
        return 0;

    int32 page = FMath::FloorToInt(XPos / PageSize.X);
    if (XPos - page * PageSize.X > PageSize.X * 0.5f)
        page++;

    return page;
}

void UScrollPane::SetPageX(int32 Value, bool bAnimation)
{
    if (!bPageMode)
        return;

    Owner->EnsureBoundsCorrect();

    if (OverlapSize.X > 0)
        SetPosX(Value * PageSize.X, bAnimation);
}

int32 UScrollPane::GetPageY() const
{
    if (!bPageMode)
        return 0;

    int32 page = FMath::FloorToInt(YPos / PageSize.Y);
    if (YPos - page * PageSize.Y > PageSize.Y * 0.5f)
        page++;

    return page;
}

void UScrollPane::SetPageY(int32 Value, bool bAnimation)
{
    if (!bPageMode)
        return;

    Owner->EnsureBoundsCorrect();

    if (OverlapSize.Y > 0)
        SetPosY(Value * PageSize.Y, bAnimation);
}

float UScrollPane::GetScrollingPosX() const
{
    return FMath::Clamp(-Container->GetPosition().X, 0.f, OverlapSize.X);
}

float UScrollPane::GetScrollingPosY() const
{
    return FMath::Clamp(-Container->GetPosition().Y, 0.f, OverlapSize.Y);
}

void UScrollPane::SetViewWidth(float Width)
{
    Width = Width + Owner->Margin.Left + Owner->Margin.Right;
    if (VtScrollBar != nullptr && !bFloating)
        Width += VtScrollBar->GetWidth();
    Owner->SetWidth(Width);
}

void UScrollPane::SetViewHeight(float Height)
{
    Height = Height + Owner->Margin.Top + Owner->Margin.Bottom;
    if (HzScrollBar != nullptr && !bFloating)
        Height += HzScrollBar->GetHeight();
    Owner->SetHeight(Height);
}

void UScrollPane::LockHeader(int32 Size)
{
    if (HeaderLockedSize == Size)
        return;

    const FVector2D& cpos = Container->GetPosition();

    HeaderLockedSize = Size;
    if (!bDispatchingPullDown && cpos.Component(RefreshBarAxis) >= 0)
    {
        TweenStart = cpos;
        TweenChange.Set(0, 0);
        TweenChange[RefreshBarAxis] = HeaderLockedSize - TweenStart.Component(RefreshBarAxis);
        TweenDuration.Set(TWEEN_TIME_DEFAULT, TWEEN_TIME_DEFAULT);
        StartTween(2);
    }
}

void UScrollPane::LockFooter(int32 Size)
{
    if (FooterLockedSize == Size)
        return;

    const FVector2D& cpos = Container->GetPosition();

    FooterLockedSize = Size;
    if (!bDispatchingPullUp && cpos.Component(RefreshBarAxis) >= 0)
    {
        TweenStart = cpos;
        TweenChange.Set(0, 0);
        float max = OverlapSize.Component(RefreshBarAxis);
        if (max == 0)
            max = FMath::Max(ContentSize.Component(RefreshBarAxis) + FooterLockedSize - ViewSize.Component(RefreshBarAxis), 0.f);
        else
            max += FooterLockedSize;
        TweenChange.Component(RefreshBarAxis) = -max - TweenStart.Component(RefreshBarAxis);
        TweenDuration.Set(TWEEN_TIME_DEFAULT, TWEEN_TIME_DEFAULT);
        StartTween(2);
    }
}

void UScrollPane::CancelDragging()
{
    if (DraggingPane.Get() == this)
        DraggingPane.Reset();

    GestureFlag = 0;
    bDragged = false;
}

void UScrollPane::HandleControllerChanged(UGController* Controller)
{
    if (PageController == Controller)
    {
        if (ScrollType == EScrollType::Horizontal)
            SetPageX(Controller->GetSelectedIndex(), true);
        else
            SetPageY(Controller->GetSelectedIndex(), true);
    }
}

void UScrollPane::UpdatePageController()
{
    if (PageController != nullptr && !PageController->bChanging)
    {
        int32 index;
        if (ScrollType == EScrollType::Horizontal)
            index = GetPageX();
        else
            index = GetPageY();
        if (index < PageController->GetPageCount())
        {
            UGController* Controller = PageController;
            PageController = nullptr; //avoid calling handleControllerChanged
            Controller->SetSelectedIndex(index);
            PageController = Controller;
        }
    }
}

void UScrollPane::AdjustMaskContainer()
{
    FVector2D Pos;
    if (bDisplayOnLeft && VtScrollBar != nullptr && !bFloating)
        Pos.X = FMath::FloorToFloat(Owner->Margin.Left + VtScrollBar->GetWidth());
    else
        Pos.X = FMath::FloorToFloat(Owner->Margin.Left);
    Pos.Y = FMath::FloorToFloat(Owner->Margin.Top);
    Pos += Owner->AlignOffset;

    MaskContainer->SetPosition(Pos);
}

void UScrollPane::OnOwnerSizeChanged()
{
    SetSize(Owner->GetSize());
    PosChanged(false);
}

void UScrollPane::SetSize(const FVector2D& InSize)
{
    if (HzScrollBar != nullptr)
    {
        HzScrollBar->SetY(InSize.Y - HzScrollBar->GetHeight());
        if (VtScrollBar != nullptr)
        {
            HzScrollBar->SetWidth(InSize.X - VtScrollBar->GetWidth() - ScrollBarMargin.Left - ScrollBarMargin.Right);
            if (bDisplayOnLeft)
                HzScrollBar->SetX(ScrollBarMargin.Left + VtScrollBar->GetWidth());
            else
                HzScrollBar->SetX(ScrollBarMargin.Left);
        }
        else
        {
            HzScrollBar->SetWidth(InSize.X - ScrollBarMargin.Left - ScrollBarMargin.Right);
            HzScrollBar->SetX(ScrollBarMargin.Left);
        }
    }
    if (VtScrollBar != nullptr)
    {
        if (!bDisplayOnLeft)
            VtScrollBar->SetX(InSize.X - VtScrollBar->GetWidth());
        if (HzScrollBar != nullptr)
            VtScrollBar->SetHeight(InSize.Y - HzScrollBar->GetHeight() - ScrollBarMargin.Top - ScrollBarMargin.Bottom);
        else
            VtScrollBar->SetHeight(InSize.Y - ScrollBarMargin.Top - ScrollBarMargin.Bottom);
        VtScrollBar->SetY(ScrollBarMargin.Top);
    }

    ViewSize = InSize;
    if (HzScrollBar != nullptr && !bFloating)
        ViewSize.Y -= HzScrollBar->GetHeight();
    if (VtScrollBar != nullptr && !bFloating)
        ViewSize.X -= VtScrollBar->GetWidth();
    ViewSize.X -= (Owner->Margin.Left + Owner->Margin.Right);
    ViewSize.Y -= (Owner->Margin.Top + Owner->Margin.Bottom);

    ViewSize.X = FMath::Max(1.f, ViewSize.X);
    ViewSize.Y = FMath::Max(1.f, ViewSize.Y);
    PageSize = ViewSize;

    AdjustMaskContainer();
    HandleSizeChanged();
}

void UScrollPane::SetContentSize(const FVector2D& InSize)
{
    if (ContentSize == InSize)
        return;

    ContentSize = InSize;
    HandleSizeChanged();
}

void UScrollPane::ChangeContentSizeOnScrolling(float DeltaWidth, float DeltaHeight, float DeltaPosX, float DeltaPosY)
{
    bool isRightmost = XPos == OverlapSize.X;
    bool isBottom = YPos == OverlapSize.Y;

    ContentSize.X += DeltaWidth;
    ContentSize.Y += DeltaHeight;
    HandleSizeChanged();

    if (Tweening == 1)
    {
        if (DeltaWidth != 0 && isRightmost && TweenChange.X < 0)
        {
            XPos = OverlapSize.X;
            TweenChange.X = -XPos - TweenStart.X;
        }

        if (DeltaHeight != 0 && isBottom && TweenChange.Y < 0)
        {
            YPos = OverlapSize.Y;
            TweenChange.Y = -YPos - TweenStart.Y;
        }
    }
    else if (Tweening == 2)
    {
        if (DeltaPosX != 0)
        {
            Container->SetX(Container->GetPosition().X - DeltaPosX);
            TweenStart.X -= DeltaPosX;
            XPos = -Container->GetPosition().X;
        }
        if (DeltaPosY != 0)
        {
            Container->SetY(Container->GetPosition().Y - DeltaPosY);
            TweenStart.Y -= DeltaPosY;
            YPos = -Container->GetPosition().Y;
        }
    }
    else if (bDragged)
    {
        if (DeltaPosX != 0)
        {
            Container->SetX(Container->GetPosition().X - DeltaPosX);
            ContainerPos.X -= DeltaPosX;
            XPos = -Container->GetPosition().X;
        }
        if (DeltaPosY != 0)
        {
            Container->SetY(Container->GetPosition().Y - DeltaPosY);
            ContainerPos.Y -= DeltaPosY;
            YPos = -Container->GetPosition().Y;
        }
    }
    else
    {
        if (DeltaWidth != 0 && isRightmost)
        {
            XPos = OverlapSize.X;
            Container->SetX(Container->GetPosition().X - XPos);
        }

        if (DeltaHeight != 0 && isBottom)
        {
            YPos = OverlapSize.Y;
            Container->SetY(Container->GetPosition().Y - YPos);
        }
    }

    if (bPageMode)
        UpdatePageController();
}

void UScrollPane::HandleSizeChanged()
{
    if (bDisplayInDemand)
    {
        bVScrollNone = ContentSize.Y <= ViewSize.Y;
        bHScrollNone = ContentSize.X <= ViewSize.X;
    }

    if (VtScrollBar != nullptr)
    {
        if (ContentSize.Y == 0)
            VtScrollBar->SetDisplayPerc(0);
        else
            VtScrollBar->SetDisplayPerc(FMath::Min(1.f, ViewSize.Y / ContentSize.Y));
    }
    if (HzScrollBar != nullptr)
    {
        if (ContentSize.X == 0)
            HzScrollBar->SetDisplayPerc(0);
        else
            HzScrollBar->SetDisplayPerc(FMath::Min(1.f, ViewSize.X / ContentSize.X));
    }

    UpdateScrollBarVisible();

    MaskContainer->SetSize(ViewSize);
    FBox2D maskRect(-Owner->AlignOffset, -Owner->AlignOffset + ViewSize);
    if (bVScrollNone && VtScrollBar != nullptr)
        maskRect.Max.X += VtScrollBar->GetWidth();
    if (bHScrollNone && HzScrollBar != nullptr)
        maskRect.Max.Y += HzScrollBar->GetHeight();
    if (bDontClipMargin)
    {
        maskRect.ShiftBy(FVector2D(-Owner->Margin.Left, -Owner->Margin.Top));
        maskRect.Max.X += Owner->Margin.Left + Owner->Margin.Right;
        maskRect.Max.Y += Owner->Margin.Top + Owner->Margin.Bottom;
    }
    MaskContainer->SetCullingBoundsExtension(FMargin(-maskRect.Min.X, -maskRect.Min.Y, maskRect.Max.X - ViewSize.X, maskRect.Max.Y - ViewSize.Y));

    if (ScrollType == EScrollType::Horizontal || ScrollType == EScrollType::Both)
        OverlapSize.X = FMath::CeilToFloat(FMath::Max(0.f, ContentSize.X - ViewSize.X));
    else
        OverlapSize.X = 0;
    if (ScrollType == EScrollType::Vertical || ScrollType == EScrollType::Both)
        OverlapSize.Y = FMath::CeilToFloat(FMath::Max(0.f, ContentSize.Y - ViewSize.Y));
    else
        OverlapSize.Y = 0;

    XPos = FMath::Clamp(XPos, 0.f, OverlapSize.X);
    YPos = FMath::Clamp(YPos, 0.f, OverlapSize.Y);
    float max = OverlapSize.Component(RefreshBarAxis);
    if (max == 0)
        max = FMath::Max(ContentSize.Component(RefreshBarAxis) + FooterLockedSize - ViewSize.Component(RefreshBarAxis), 0.f);
    else
        max += FooterLockedSize;
    const FVector2D& Pos = Container->GetPosition();
    if (RefreshBarAxis == 0)
        Container->SetPosition(FVector2D(FMath::Clamp(Pos.X, -max, HeaderLockedSize),
            FMath::Clamp(Pos.Y, -OverlapSize.Y, 0.f)));
    else
        Container->SetPosition(FVector2D(FMath::Clamp(Pos.X, -OverlapSize.X, 0.f),
            FMath::Clamp(Pos.Y, -max, HeaderLockedSize)));

    if (Header != nullptr)
    {
        if (RefreshBarAxis == 0)
            Header->SetHeight(ViewSize.Y);
        else
            Header->SetWidth(ViewSize.X);
    }

    if (Footer != nullptr)
    {
        if (RefreshBarAxis == 0)
            Footer->SetHeight(ViewSize.Y);
        else
            Footer->SetWidth(ViewSize.X);
    }

    UpdateScrollBarPos();
    if (bPageMode)
        UpdatePageController();
}

void UScrollPane::PosChanged(bool bAnimation)
{
    if (AniFlag == 0)
        AniFlag = bAnimation ? 1 : -1;
    else if (AniFlag == 1 && !bAnimation)
        AniFlag = -1;

    bNeedRefresh = true;
    Owner->GetApp()->DelayCall(RefreshTimerHandle, this, &UScrollPane::Refresh);
}

void UScrollPane::Refresh()
{
    Owner->GetApp()->CancelDelayCall(RefreshTimerHandle);

    bNeedRefresh = false;

    if (bPageMode || bSnapToItem)
    {
        FVector2D pos(-XPos, -YPos);
        AlignPosition(pos, false);
        XPos = -pos.X;
        YPos = -pos.Y;
    }

    Refresh2();

    Owner->DispatchEvent(FUIEvents::Scroll);
    if (bNeedRefresh) //pos may change in onScroll
    {
        bNeedRefresh = false;
        Owner->GetApp()->CancelDelayCall(RefreshTimerHandle);

        Refresh2();
    }

    UpdateScrollBarPos();
    AniFlag = 0;
}

void UScrollPane::Refresh2()
{
    if (AniFlag == 1 && !bDragged)
    {
        FVector2D pos;

        if (OverlapSize.X > 0)
            pos.X = -(int32)XPos;
        else
        {
            if (Container->GetPosition().X != 0)
                Container->SetX(0);
            pos.X = 0;
        }
        if (OverlapSize.Y > 0)
            pos.Y = -(int32)YPos;
        else
        {
            if (Container->GetPosition().Y != 0)
                Container->SetY(0);
            pos.Y = 0;
        }

        if (pos != Container->GetPosition())
        {
            TweenDuration.Set(TWEEN_TIME_GO, TWEEN_TIME_GO);
            TweenStart = Container->GetPosition();
            TweenChange = pos - TweenStart;
            StartTween(1);
        }
        else if (Tweening != 0)
            KillTween();
    }
    else
    {
        if (Tweening != 0)
            KillTween();

        Container->SetPosition(FVector2D((int32)-XPos, (int32)-YPos));

        LoopCheckingCurrent();
    }

    if (bPageMode)
        UpdatePageController();
}

void UScrollPane::UpdateScrollBarPos()
{
    if (VtScrollBar != nullptr)
        VtScrollBar->SetScrollPerc(OverlapSize.Y == 0 ? 0 : FMath::Clamp(-Container->GetPosition().Y, 0.f, OverlapSize.Y) / OverlapSize.Y);

    if (HzScrollBar != nullptr)
        HzScrollBar->SetScrollPerc(OverlapSize.X == 0 ? 0 : FMath::Clamp(-Container->GetPosition().X, 0.f, OverlapSize.X) / OverlapSize.X);

    CheckRefreshBar();
}

void UScrollPane::UpdateScrollBarVisible()
{
    if (VtScrollBar != nullptr)
    {
        if (ViewSize.Y <= VtScrollBar->GetMinSize() || bVScrollNone)
            VtScrollBar->SetVisible(false);
        else
            UpdateScrollBarVisible2(VtScrollBar);
    }

    if (HzScrollBar != nullptr)
    {
        if (ViewSize.X <= HzScrollBar->GetMinSize() || bHScrollNone)
            HzScrollBar->SetVisible(false);
        else
            UpdateScrollBarVisible2(HzScrollBar);
    }
}

void UScrollPane::UpdateScrollBarVisible2(UGScrollBar* Bar)
{
    if (bScrollBarDisplayAuto)
        FGTween::Kill(Bar, false);

    if (bScrollBarDisplayAuto && !bHover && Tweening == 0 && !bDragged && !Bar->bGripDragging)
    {
        if (Bar->IsVisible())
            FGTween::To(1, 0, 0.5f)
            ->SetDelay(0.5f)
            ->OnUpdate(FTweenDelegate::CreateStatic(&FGTweenAction::SetAlpha))
            ->OnComplete(FTweenDelegate::CreateUObject(this, &UScrollPane::OnBarTweenComplete))
            ->SetTarget(Bar);
    }
    else
    {
        Bar->SetAlpha(1);
        Bar->SetVisible(true);
    }
}

void UScrollPane::OnBarTweenComplete(FGTweener* Tweener)
{
    UGObject* bar = (UGObject*)Tweener->GetTarget();
    bar->SetAlpha(1);
    bar->SetVisible(false);
}

float UScrollPane::GetLoopPartSize(float Division, int32 Axis)
{
    return (ContentSize.Component(Axis) + (Axis == 0 ? ((UGList*)Owner)->GetColumnGap() : ((UGList*)Owner)->GetLineGap())) / Division;
}

bool UScrollPane::LoopCheckingCurrent()
{
    bool changed = false;
    if (LoopMode == 1 && OverlapSize.X > 0)
    {
        if (XPos < 0.001f)
        {
            XPos += GetLoopPartSize(2, 0);
            changed = true;
        }
        else if (XPos >= OverlapSize.X)
        {
            XPos -= GetLoopPartSize(2, 0);
            changed = true;
        }
    }
    else if (LoopMode == 2 && OverlapSize.Y > 0)
    {
        if (YPos < 0.001f)
        {
            YPos += GetLoopPartSize(2, 1);
            changed = true;
        }
        else if (YPos >= OverlapSize.Y)
        {
            YPos -= GetLoopPartSize(2, 1);
            changed = true;
        }
    }

    if (changed)
        Container->SetPosition(FVector2D((int32)-XPos, (int32)-YPos));

    return changed;
}

void UScrollPane::LoopCheckingTarget(FVector2D& EndPos)
{
    if (LoopMode == 1)
        LoopCheckingTarget(EndPos, 0);

    if (LoopMode == 2)
        LoopCheckingTarget(EndPos, 1);
}

void UScrollPane::LoopCheckingTarget(FVector2D& EndPos, int32 Axis)
{
    if (EndPos.Component(Axis) > 0)
    {
        float halfSize = GetLoopPartSize(2, Axis);
        float tmp = TweenStart.Component(Axis) - halfSize;
        if (tmp <= 0 && tmp >= -OverlapSize.Component(Axis))
        {
            EndPos.Component(Axis) = -halfSize;
            TweenStart.Component(Axis) = tmp;
        }
    }
    else if (EndPos.Component(Axis) < -OverlapSize.Component(Axis))
    {
        float halfSize = GetLoopPartSize(2, Axis);
        float tmp = TweenStart.Component(Axis) + halfSize;
        if (tmp <= 0 && tmp >= -OverlapSize.Component(Axis))
        {
            EndPos.Component(Axis) += halfSize;
            TweenStart.Component(Axis) += tmp;
        }
    }
}

void UScrollPane::LoopCheckingNewPos(float& Value, int32 Axis)
{
    float overlapSize = OverlapSize.Component(Axis);
    if (overlapSize == 0)
        return;

    float pos = Axis == 0 ? XPos : YPos;
    bool changed = false;
    if (Value < 0.001f)
    {
        Value += GetLoopPartSize(2, Axis);
        if (Value > pos)
        {
            float v = GetLoopPartSize(6, Axis);
            v = FMath::CeilToFloat((Value - pos) / v) * v;
            pos = FMath::Clamp(pos + v, 0.f, overlapSize);
            changed = true;
        }
    }
    else if (Value >= overlapSize)
    {
        Value -= GetLoopPartSize(2, Axis);
        if (Value < pos)
        {
            float v = GetLoopPartSize(6, Axis);
            v = FMath::CeilToFloat((pos - Value) / v) * v;
            pos = FMath::Clamp(pos - v, 0.f, overlapSize);
            changed = true;
        }
    }

    if (changed)
    {
        if (Axis == 0)
            Container->SetX(-(int32)pos);
        else
            Container->SetY(-(int32)pos);
    }
}

void UScrollPane::AlignPosition(FVector2D& Pos, bool bInertialScrolling)
{
    if (bPageMode)
    {
        Pos.X = AlignByPage(Pos.X, 0, bInertialScrolling);
        Pos.Y = AlignByPage(Pos.Y, 1, bInertialScrolling);
    }
    else if (bSnapToItem)
    {
        FVector2D tmp = Owner->GetSnappingPosition(-Pos);
        if (Pos.X < 0 && Pos.X > -OverlapSize.X)
            Pos.X = -tmp.X;
        if (Pos.Y < 0 && Pos.Y > -OverlapSize.Y)
            Pos.Y = -tmp.Y;
    }
}

float UScrollPane::AlignByPage(float Pos, int32 Axis, bool bInertialScrolling)
{
    int32 page;
    float pageSize = PageSize.Component(Axis);
    float overlapSize = OverlapSize.Component(Axis);
    float contentSize = ContentSize.Component(Axis);

    if (Pos > 0)
        page = 0;
    else if (Pos < -overlapSize)
        page = FMath::CeilToFloat(contentSize / pageSize) - 1;
    else
    {
        page = FMath::FloorToInt(-Pos / pageSize);
        float change = bInertialScrolling ? (Pos - ContainerPos.Component(Axis)) : (Pos - Container->GetPosition().Component(Axis));
        float testPageSize = FMath::Min(pageSize, contentSize - (page + 1) * pageSize);
        float delta = -Pos - page * pageSize;

        if (FMath::Abs(change) > pageSize)
        {
            if (delta > testPageSize * 0.5f)
                page++;
        }
        else
        {
            if (delta > testPageSize * (change < 0 ? 0.3f : 0.7f))
                page++;
        }

        Pos = -page * pageSize;
        if (Pos < -overlapSize)
            Pos = -overlapSize;
    }

    if (bInertialScrolling)
    {
        float oldPos = TweenStart.Component(Axis);
        int32 oldPage;
        if (oldPos > 0)
            oldPage = 0;
        else if (oldPos < -overlapSize)
            oldPage = FMath::CeilToInt(contentSize / pageSize) - 1;
        else
            oldPage = FMath::FloorToInt(-oldPos / pageSize);
        int32 startPage = FMath::FloorToInt(-ContainerPos.Component(Axis) / pageSize);
        if (FMath::Abs(page - startPage) > 1 && FMath::Abs(oldPage - startPage) <= 1)
        {
            if (page > startPage)
                page = startPage + 1;
            else
                page = startPage - 1;
            Pos = -page * pageSize;
        }
    }

    return Pos;
}

FVector2D UScrollPane::UpdateTargetAndDuration(const FVector2D& OrignPos)
{
    FVector2D ret(0, 0);
    ret.X = UpdateTargetAndDuration(OrignPos.X, 0);
    ret.Y = UpdateTargetAndDuration(OrignPos.Y, 1);
    return ret;
}

float UScrollPane::UpdateTargetAndDuration(float Pos, int32 Axis)
{
    float v = Velocity.Component(Axis);
    float duration = 0;

    if (Pos > 0)
        Pos = 0;
    else if (Pos < -OverlapSize.Component(Axis))
        Pos = -OverlapSize.Component(Axis);
    else
    {
        float v2 = FMath::Abs(v) * VelocityScale;
        float ratio = 0;
        if (FPlatformMisc::DesktopTouchScreen())
        {
            if (v2 > 500)
                ratio = FMath::Pow((v2 - 500) / 500, 2);
        }
        else
        {
            FVector2D winSize;
            GWorld->GetGameViewport()->GetViewportSize(winSize);
            v2 *= 1136.0f / FMath::Max(winSize.X, winSize.Y);

            if (bPageMode)
            {
                if (v2 > 500)
                    ratio = FMath::Pow((v2 - 500) / 500, 2);
            }
            else
            {
                if (v2 > 1000)
                    ratio = FMath::Pow((v2 - 1000) / 1000, 2);
            }
        }

        if (ratio != 0)
        {
            if (ratio > 1)
                ratio = 1;

            v2 *= ratio;
            v *= ratio;
            Velocity.Component(Axis) = v;

            duration = FMath::Loge(60 / v2) / FMath::Loge(DecelerationRate) / 60;
            float change = FMath::FloorToFloat(v * duration * 0.4f);
            Pos += change;
        }
    }

    if (duration < TWEEN_TIME_DEFAULT)
        duration = TWEEN_TIME_DEFAULT;
    TweenDuration.Component(Axis) = duration;

    return Pos;
}

void UScrollPane::FixDuration(int32 Axis, float OldChange)
{
    float tweenChange = TweenChange.Component(Axis);
    if (tweenChange == 0 || FMath::Abs(tweenChange) >= FMath::Abs(OldChange))
        return;

    float newDuration = FMath::Abs(tweenChange / OldChange) * TweenDuration.Component(Axis);
    if (newDuration < TWEEN_TIME_DEFAULT)
        newDuration = TWEEN_TIME_DEFAULT;

    TweenDuration.Component(Axis) = newDuration;
}

void UScrollPane::StartTween(int32 Type)
{
    TweenTime.Set(0, 0);
    Tweening = Type;
    GWorld->GetTimerManager().SetTimer(TickTimerHandle,
        FTimerDelegate::CreateUObject(this, &UScrollPane::TweenUpdate),
        0.016f,
        true);
    UpdateScrollBarVisible();
}

void UScrollPane::KillTween()
{
    if (Tweening == 1)
    {
        FVector2D t = TweenStart + TweenChange;
        Container->SetPosition(t);
        Owner->DispatchEvent(FUIEvents::Scroll);
    }

    Tweening = 0;
    GWorld->GetTimerManager().ClearTimer(TickTimerHandle);
    Owner->DispatchEvent(FUIEvents::ScrollEnd);
}

void UScrollPane::CheckRefreshBar()
{
    if (Header == nullptr && Footer == nullptr)
        return;

    float pos = Container->GetPosition().Component(RefreshBarAxis);
    if (Header != nullptr)
    {
        if (pos > 0)
        {
            Header->SetVisible(true);
            FVector2D vec;

            vec = Header->GetSize();
            vec.Component(RefreshBarAxis) = pos;
            Header->SetSize(vec);
        }
        else
            Header->SetVisible(false);
    }

    if (Footer != nullptr)
    {
        float max = OverlapSize.Component(RefreshBarAxis);
        if (pos < -max || (max == 0 && FooterLockedSize > 0))
        {
            Footer->SetVisible(true);

            FVector2D vec;

            vec = Footer->GetPosition();
            if (max > 0)
                vec.Component(RefreshBarAxis) = pos + ContentSize.Component(RefreshBarAxis);
            else
                vec.Component(RefreshBarAxis) = FMath::Max(FMath::Min(pos + ViewSize.Component(RefreshBarAxis), ViewSize.Component(RefreshBarAxis) - FooterLockedSize), ViewSize.Component(RefreshBarAxis) - ContentSize.Component(RefreshBarAxis));
            Footer->SetPosition(vec);

            vec = Footer->GetSize();
            if (max > 0)
                vec.Component(RefreshBarAxis) = -max - pos;
            else
                vec.Component(RefreshBarAxis) = ViewSize.Component(RefreshBarAxis) - Footer->GetPosition().Component(RefreshBarAxis);
            Footer->SetSize(vec);
        }
        else
            Footer->SetVisible(false);
    }
}

void UScrollPane::TweenUpdate()
{
    float dt = GWorld->GetTimerManager().GetTimerElapsed(TickTimerHandle);

    float nx = RunTween(0, dt);
    float ny = RunTween(1, dt);

    Container->SetPosition(FVector2D(nx, ny));

    if (Tweening == 2)
    {
        if (OverlapSize.X > 0)
            XPos = FMath::Clamp(-nx, 0.f, OverlapSize.X);
        if (OverlapSize.Y > 0)
            YPos = FMath::Clamp(-ny, 0.f, OverlapSize.Y);

        if (bPageMode)
            UpdatePageController();
    }

    if (TweenChange.X == 0 && TweenChange.Y == 0)
    {
        Tweening = 0;
        GWorld->GetTimerManager().ClearTimer(TickTimerHandle);

        LoopCheckingCurrent();

        UpdateScrollBarPos();
        UpdateScrollBarVisible();

        Owner->DispatchEvent(FUIEvents::Scroll);
        Owner->DispatchEvent(FUIEvents::ScrollEnd);
    }
    else
    {
        UpdateScrollBarPos();
        Owner->DispatchEvent(FUIEvents::Scroll);
    }
}

float UScrollPane::RunTween(int32 Axis, float DeltaTime)
{
    float newValue;
    if (TweenChange.Component(Axis) != 0)
    {
        TweenTime.Component(Axis) += DeltaTime;
        if (TweenTime.Component(Axis) >= TweenDuration.Component(Axis))
        {
            newValue = TweenStart.Component(Axis) + TweenChange.Component(Axis);
            TweenChange.Component(Axis) = 0;
        }
        else
        {
            float ratio = sp_EaseFunc(TweenTime.Component(Axis), TweenDuration.Component(Axis));
            newValue = TweenStart.Component(Axis) + (int32)(TweenChange.Component(Axis) * ratio);
        }

        float threshold1 = 0;
        float threshold2 = -OverlapSize.Component(Axis);
        if (HeaderLockedSize > 0 && RefreshBarAxis == Axis)
            threshold1 = HeaderLockedSize;
        if (FooterLockedSize > 0 && RefreshBarAxis == Axis)
        {
            float max = OverlapSize.Component(RefreshBarAxis);
            if (max == 0)
                max = FMath::Max(ContentSize.Component(RefreshBarAxis) + FooterLockedSize - ViewSize.Component(RefreshBarAxis), 0.f);
            else
                max += FooterLockedSize;
            threshold2 = -max;
        }

        if (Tweening == 2 && bBouncebackEffect)
        {
            if ((newValue > 20 + threshold1 && TweenChange.Component(Axis) > 0) || (newValue > threshold1 && TweenChange.Component(Axis) == 0))
            {
                TweenTime.Component(Axis) = 0;
                TweenDuration.Component(Axis) = TWEEN_TIME_DEFAULT;
                TweenChange.Component(Axis) = -newValue + threshold1;
                TweenStart.Component(Axis) = newValue;
            }
            else if ((newValue < threshold2 - 20 && TweenChange.Component(Axis) < 0) || (newValue < threshold2 && TweenChange.Component(Axis) == 0))
            {
                TweenTime.Component(Axis) = 0;
                TweenDuration.Component(Axis) = TWEEN_TIME_DEFAULT;
                TweenChange.Component(Axis) = threshold2 - newValue;
                TweenStart.Component(Axis) = newValue;
            }
        }
        else
        {
            if (newValue > threshold1)
            {
                newValue = threshold1;
                TweenChange.Component(Axis) = 0;
            }
            else if (newValue < threshold2)
            {
                newValue = threshold2;
                TweenChange.Component(Axis) = 0;
            }
        }
    }
    else
        newValue = Container->GetPosition().Component(Axis);

    return newValue;
}

void UScrollPane::OnTouchBegin(UEventContext* Context)
{
    if (!bTouchEffect)
        return;

    Context->CaptureTouch();
    FVector2D pt = Owner->GlobalToLocal(Context->GetPointerPosition());

    if (Tweening != 0)
    {
        KillTween();
        Owner->GetApp()->CancelClick(Context->GetUserIndex(), Context->GetPointerIndex());

        bDragged = true;
    }
    else
        bDragged = false;

    ContainerPos = Container->GetPosition();
    BeginTouchPos = LastTouchPos = pt;
    LastTouchGlobalPos = Context->GetPointerPosition();
    bIsHoldAreaDone = false;
    Velocity.Set(0, 0);
    VelocityScale = 1;
    LastMoveTime = GWorld->GetTimeSeconds();
}

void UScrollPane::OnTouchMove(UEventContext* Context)
{
    if (!bTouchEffect)
        return;

    if ((DraggingPane.IsValid() && DraggingPane.Get() != this) || UGObject::GetDraggingObject() != nullptr)
        return;

    FVector2D pt = Owner->GlobalToLocal(Context->GetPointerPosition());

    int32 sensitivity;
    if (FPlatformMisc::DesktopTouchScreen())
        sensitivity = 8;
    else
        sensitivity = FUIConfig::Config.TouchScrollSensitivity;

    float diff;
    bool sv = false, sh = false;

    if (ScrollType == EScrollType::Vertical)
    {
        if (!bIsHoldAreaDone)
        {
            GestureFlag |= 1;

            diff = FMath::Abs(BeginTouchPos.Y - pt.Y);
            if (diff < sensitivity)
                return;

            if ((GestureFlag & 2) != 0)
            {
                float diff2 = FMath::Abs(BeginTouchPos.X - pt.X);
                if (diff < diff2)
                    return;
            }
        }

        sv = true;
    }
    else if (ScrollType == EScrollType::Horizontal)
    {
        if (!bIsHoldAreaDone)
        {
            GestureFlag |= 2;

            diff = FMath::Abs(BeginTouchPos.X - pt.X);
            if (diff < sensitivity)
                return;

            if ((GestureFlag & 1) != 0)
            {
                float diff2 = FMath::Abs(BeginTouchPos.Y - pt.Y);
                if (diff < diff2)
                    return;
            }
        }

        sh = true;
    }
    else
    {
        GestureFlag = 3;

        if (!bIsHoldAreaDone)
        {
            diff = FMath::Abs(BeginTouchPos.Y - pt.Y);
            if (diff < sensitivity)
            {
                diff = FMath::Abs(BeginTouchPos.X - pt.X);
                if (diff < sensitivity)
                    return;
            }
        }

        sv = sh = true;
    }

    FVector2D newPos = (ContainerPos + pt - BeginTouchPos).RoundToVector();

    if (sv)
    {
        if (newPos.Y > 0)
        {
            if (!bBouncebackEffect)
                Container->SetY(0);
            else if (Header != nullptr && Header->MaxSize.Y != 0)
                Container->SetY(((int32)FMath::Min(newPos.Y * 0.5f, Header->MaxSize.Y)));
            else
                Container->SetY(((int32)FMath::Min(newPos.Y * 0.5f, ViewSize.Y * PULL_RATIO)));
        }
        else if (newPos.Y < -OverlapSize.Y)
        {
            if (!bBouncebackEffect)
                Container->SetY(-OverlapSize.Y);
            else if (Footer != nullptr && Footer->MaxSize.Y > 0)
                Container->SetY(((int32)FMath::Max((newPos.Y + OverlapSize.Y) * 0.5f, -Footer->MaxSize.Y) - OverlapSize.Y));
            else
                Container->SetY(((int32)FMath::Max((newPos.Y + OverlapSize.Y) * 0.5f, -ViewSize.Y * PULL_RATIO) - OverlapSize.Y));
        }
        else
            Container->SetY(newPos.Y);
    }

    if (sh)
    {
        if (newPos.X > 0)
        {
            if (!bBouncebackEffect)
                Container->SetX(0);
            else if (Header != nullptr && Header->MaxSize.X != 0)
                Container->SetX((int32)FMath::Min(newPos.X * 0.5f, Header->MaxSize.X));
            else
                Container->SetX((int32)FMath::Min(newPos.X * 0.5f, ViewSize.X * PULL_RATIO));
        }
        else if (newPos.X < 0 - OverlapSize.X)
        {
            if (!bBouncebackEffect)
                Container->SetX(-OverlapSize.X);
            else if (Footer != nullptr && Footer->MaxSize.X > 0)
                Container->SetX((int32)FMath::Max((newPos.X + OverlapSize.X) * 0.5f, -Footer->MaxSize.X) - OverlapSize.X);
            else
                Container->SetX((int32)FMath::Max((newPos.X + OverlapSize.X) * 0.5f, -ViewSize.X * PULL_RATIO) - OverlapSize.X);
        }
        else
            Container->SetX(newPos.X);
    }

    float deltaTime = FSlateApplication::Get().GetDeltaTime();// GWorld->GetDeltaSeconds();
    float elapsed = GWorld->GetTimeSeconds() - LastMoveTime;
    elapsed = elapsed * 60 - 1;
    if (elapsed > 1)
        Velocity *= FMath::Pow(0.833f, elapsed);
    FVector2D deltaPosition = pt - LastTouchPos;
    if (!sh)
        deltaPosition.X = 0;
    if (!sv)
        deltaPosition.Y = 0;
    Velocity = FMath::Lerp(Velocity, deltaPosition / deltaTime, deltaTime * 10);

    FVector2D deltaGlobalPosition = LastTouchGlobalPos - Context->GetPointerPosition();
    if (deltaPosition.X != 0)
        VelocityScale = FMath::Abs(deltaGlobalPosition.X / deltaPosition.X);
    else if (deltaPosition.Y != 0)
        VelocityScale = FMath::Abs(deltaGlobalPosition.Y / deltaPosition.Y);

    LastTouchPos = pt;
    LastTouchGlobalPos = Context->GetPointerPosition();
    LastMoveTime = GWorld->GetTimeSeconds();

    if (OverlapSize.X > 0)
        XPos = FMath::Clamp(-Container->GetPosition().X, 0.f, OverlapSize.X);
    if (OverlapSize.Y > 0)
        YPos = FMath::Clamp(-Container->GetPosition().Y, 0.f, OverlapSize.Y);

    if (LoopMode != 0)
    {
        newPos = Container->GetPosition();
        if (LoopCheckingCurrent())
            ContainerPos += Container->GetPosition() - newPos;
    }

    DraggingPane = this;
    bIsHoldAreaDone = true;
    bDragged = true;

    UpdateScrollBarPos();
    UpdateScrollBarVisible();
    if (bPageMode)
        UpdatePageController();

    Owner->DispatchEvent(FUIEvents::Scroll);
}

void UScrollPane::OnTouchEnd(UEventContext* Context)
{
    if (DraggingPane.Get() == this)
        DraggingPane.Reset();

    GestureFlag = 0;

    if (!bDragged || !bTouchEffect)
    {
        bDragged = false;
        return;
    }

    bDragged = false;
    TweenStart = Container->GetPosition();

    FVector2D endPos = TweenStart;
    bool flag = false;
    if (Container->GetPosition().X > 0)
    {
        endPos.X = 0;
        flag = true;
    }
    else if (Container->GetPosition().X < -OverlapSize.X)
    {
        endPos.X = -OverlapSize.X;
        flag = true;
    }
    if (Container->GetPosition().Y > 0)
    {
        endPos.Y = 0;
        flag = true;
    }
    else if (Container->GetPosition().Y < -OverlapSize.Y)
    {
        endPos.Y = -OverlapSize.Y;
        flag = true;
    }

    if (flag)
    {
        TweenChange = endPos - TweenStart;
        if (TweenChange.X < -FUIConfig::Config.TouchDragSensitivity || TweenChange.Y < -FUIConfig::Config.TouchDragSensitivity)
            Owner->DispatchEvent(FUIEvents::PullDownRelease);
        else if (TweenChange.X > FUIConfig::Config.TouchDragSensitivity || TweenChange.Y > FUIConfig::Config.TouchDragSensitivity)
            Owner->DispatchEvent(FUIEvents::PullUpRelease);

        if (HeaderLockedSize > 0 && endPos.Component(RefreshBarAxis) == 0)
        {
            endPos.Component(RefreshBarAxis) = HeaderLockedSize;
            TweenChange = endPos - TweenStart;
        }
        else if (FooterLockedSize > 0 && endPos.Component(RefreshBarAxis) == -OverlapSize.Component(RefreshBarAxis))
        {
            float max = OverlapSize.Component(RefreshBarAxis);
            if (max == 0)
                max = FMath::Max(ContentSize.Component(RefreshBarAxis) + FooterLockedSize - ViewSize.Component(RefreshBarAxis), 0.f);
            else
                max += FooterLockedSize;
            endPos.Component(RefreshBarAxis) = -max;
            TweenChange = endPos - TweenStart;
        }

        TweenDuration.Set(TWEEN_TIME_DEFAULT, TWEEN_TIME_DEFAULT);
    }
    else
    {
        if (!bInertiaDisabled)
        {
            float elapsed = GWorld->GetTimeSeconds() - LastMoveTime;
            elapsed = elapsed * 60 - 1;
            if (elapsed > 1)
                Velocity *= FMath::Pow(0.833f, elapsed);

            endPos = UpdateTargetAndDuration(TweenStart);
        }
        else
            TweenDuration.Set(TWEEN_TIME_DEFAULT, TWEEN_TIME_DEFAULT);
        FVector2D oldChange = endPos - TweenStart;

        LoopCheckingTarget(endPos);
        if (bPageMode || bSnapToItem)
            AlignPosition(endPos, true);

        TweenChange = endPos - TweenStart;
        if (TweenChange.X == 0 && TweenChange.Y == 0)
        {
            UpdateScrollBarVisible();
            return;
        }

        if (bPageMode || bSnapToItem)
        {
            FixDuration(0, oldChange.X);
            FixDuration(1, oldChange.Y);
        }
    }

    StartTween(2);
}

void UScrollPane::OnMouseWheel(UEventContext* Context)
{
    if (!bMouseWheelEnabled)
        return;

    float delta = -Context->GetPointerEvent().GetWheelDelta();
    if (bSnapToItem && FMath::Abs(delta) < 1)
        delta = FMath::Sign(delta);

    if (OverlapSize.X > 0 && OverlapSize.Y == 0)
    {
        float step = bPageMode ? PageSize.X : ScrollStep;
        SetPosX(XPos + step * delta, false);
    }
    else
    {
        float step = bPageMode ? PageSize.Y : ScrollStep;
        SetPosY(YPos + step * delta, false);
    }
}

void UScrollPane::OnRollOver(UEventContext* Context)
{
    bHover = true;
    UpdateScrollBarVisible();
}

void UScrollPane::OnRollOut(UEventContext* Context)
{
    bHover = false;
    UpdateScrollBarVisible();
}