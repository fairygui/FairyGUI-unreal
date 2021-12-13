#include "UI/GComponent.h"
#include "UI/GButton.h"
#include "UI/GGroup.h"
#include "UI/Relations.h"
#include "UI/TranslationHelper.h"
#include "UI/UIObjectFactory.h"
#include "UI/UIPackage.h"
#include "UI/GController.h"
#include "UI/Transition.h"
#include "UI/GRoot.h"
#include "Utils/ByteBuffer.h"
#include "Widgets/SContainer.h"
#include "Widgets/HitTest.h"
#include "Tween/GTween.h"
#include "FairyApplication.h"


UGComponent::UGComponent() :
    AlignOffset(ForceInit)
{
    DisplayObject = RootContainer = SNew(SContainer).GObject(this);
    DisplayObject->SetOpaque(false);

    Container = SNew(SContainer);
    Container->SetOpaque(false);
    RootContainer->AddChild(Container.ToSharedRef());
}

UGComponent::~UGComponent()
{
}

UGObject* UGComponent::AddChild(UGObject* Child)
{
    AddChildAt(Child, Children.Num());
    return Child;
}

UGObject* UGComponent::AddChildAt(UGObject* Child, int32 Index)
{
    verifyf(Child != nullptr, TEXT("Argument must be non-nil"));
    verifyf(Index >= 0 && Index <= Children.Num(), TEXT("Invalid child index"));

    if (Child->Parent == this)
    {
        SetChildIndex(Child, Index);
    }
    else
    {
        Child->RemoveFromParent();
        Child->Parent = this;

        int32 cnt = Children.Num();
        if (Child->SortingOrder != 0)
        {
            SortingChildCount++;
            Index = GetInsertPosForSortingChild(Child);
        }
        else if (SortingChildCount > 0)
        {
            if (Index > (cnt - SortingChildCount))
                Index = cnt - SortingChildCount;
        }

        if (Index == cnt)
            Children.Add(Child);
        else
            Children.Insert(Child, Index);

        ChildStateChanged(Child);
        SetBoundsChangedFlag();
    }
    return Child;
}

int32 UGComponent::GetInsertPosForSortingChild(UGObject* Child)
{
    int32 cnt = Children.Num();
    int32 i;
    for (i = 0; i < cnt; i++)
    {
        UGObject* Obj = Children[i];
        if (Obj == Child)
            continue;

        if (Child->SortingOrder < Obj->SortingOrder)
            break;
    }
    return i;
}

void UGComponent::RemoveChild(UGObject* Child)
{
    verifyf(Child != nullptr, TEXT("Argument must be non-nil"));

    int32 ChildIndex = Children.Find(Child);
    if (ChildIndex != INDEX_NONE)
        RemoveChildAt(ChildIndex);
}

void UGComponent::RemoveChildAt(int32 Index)
{
    verifyf(Index >= 0 && Index < Children.Num(), TEXT("Invalid child index"));

    UGObject* Child = Children[Index];

    Child->Parent = nullptr;

    if (Child->SortingOrder != 0)
        SortingChildCount--;

    Child->SetGroup(nullptr);
    if (Child->DisplayObject->GetParentWidget().IsValid())
    {
        Container->RemoveChild(Child->DisplayObject.ToSharedRef());
        if (ChildrenRenderOrder == EChildrenRenderOrder::Arch)
            BuildNativeDisplayList();
    }

    Children.RemoveAt(Index);
    SetBoundsChangedFlag();
}

void UGComponent::RemoveChildren(int32 BeginIndex, int32 EndIndex)
{
    if (EndIndex < 0 || EndIndex >= Children.Num())
        EndIndex = Children.Num() - 1;

    for (int32 i = BeginIndex; i <= EndIndex; ++i)
        RemoveChildAt(BeginIndex);
}

UGObject* UGComponent::GetChildAt(int32 Index, TSubclassOf<UGObject> ClassType) const
{
    verifyf(Index >= 0 && Index < Children.Num(), TEXT("Invalid child index"));

    return Children[Index];
}

UGObject* UGComponent::GetChild(const FString& ChildName, TSubclassOf<UGObject> ClassType) const
{
    for (const auto& Child : Children)
    {
        if (Child->Name.Compare(ChildName) == 0)
            return Child;
    }

    return nullptr;
}

UGObject* UGComponent::GetChildByPath(const FString& Path, TSubclassOf<UGObject> ClassType) const
{
    const UGComponent* Com = this;
    UGObject* Obj = nullptr;

    int32 Index1 = 0, Index2 = -1;
    while ((Index2 = Path.Find(TEXT("."), ESearchCase::IgnoreCase, ESearchDir::FromStart, Index1)) != -1
        || Index1 == 0)
    {
        if (Index2 == -1)
            Index2 = Path.Len();

        if (Com == nullptr)
        {
            Com = Cast<UGComponent>(Obj);
            if (Com == nullptr)
            {
                Obj = nullptr;
                break;
            }
        }

        Obj = Com->GetChild(Path.Mid(Index1, Index2 - Index1));
        if (!Obj)
            break;

        Com = nullptr;
        Index1 = Index2 + 1;
    }

    return Obj;
}

UGObject* UGComponent::GetChildInGroup(const UGGroup* InGroup, const FString& ChildName, TSubclassOf<UGObject> ClassType) const
{
    verifyf(InGroup != nullptr, TEXT("Argument must be non-nil"));

    for (const auto& Obj : Children)
    {
        if (Obj->GetGroup() == InGroup && Obj->Name.Compare(ChildName) == 0)
            return Obj;
    }

    return nullptr;
}

UGObject* UGComponent::GetChildByID(const FString& ChildID) const
{
    for (const auto& Obj : Children)
    {
        if (Obj->ID.Compare(ChildID) == 0)
            return Obj;
    }

    return nullptr;
}

int32 UGComponent::GetChildIndex(const UGObject* Child) const
{
    verifyf(Child != nullptr, TEXT("Argument must be non-nil"));

    return Children.IndexOfByKey(Child);
}

void UGComponent::SetChildIndex(UGObject* Child, int32 Index)
{
    verifyf(Child != nullptr, TEXT("Argument must be non-nil"));

    int32 OldIndex = Children.Find(Child);
    verifyf(OldIndex != -1, TEXT("Not a child of this container"));

    if (Child->SortingOrder != 0) //no effect
        return;

    int32 cnt = Children.Num();
    if (SortingChildCount > 0)
    {
        if (Index > (cnt - SortingChildCount - 1))
            Index = cnt - SortingChildCount - 1;
    }

    MoveChild(Child, OldIndex, Index);
}

int UGComponent::SetChildIndexBefore(UGObject* Child, int32 Index)
{
    verifyf(Child != nullptr, TEXT("Argument must be non-nil"));

    int32 OldIndex = Children.Find(Child);
    verifyf(OldIndex != -1, TEXT("Not a child of this container"));

    if (Child->SortingOrder != 0) //no effect
        return OldIndex;

    int32 cnt = Children.Num();
    if (SortingChildCount > 0)
    {
        if (Index > (cnt - SortingChildCount - 1))
            Index = cnt - SortingChildCount - 1;
    }

    if (OldIndex < Index)
        return MoveChild(Child, OldIndex, Index - 1);
    else
        return MoveChild(Child, OldIndex, Index);
}

int32 UGComponent::MoveChild(UGObject* Child, int32 OldIndex, int32 Index)
{
    int32 cnt = Children.Num();
    if (Index > cnt)
        Index = cnt;

    if (OldIndex == Index)
        return OldIndex;

    Children.RemoveAt(OldIndex);
    if (Index >= cnt)
        Children.Add(Child);
    else
        Children.Insert(Child, Index);

    if (Child->DisplayObject->IsParentValid())
    {
        int32 DisplayIndex = 0;
        if (ChildrenRenderOrder == EChildrenRenderOrder::Ascent)
        {
            for (int32 i = 0; i < Index; i++)
            {
                UGObject* Obj = Children[i];
                if (Obj->DisplayObject->IsParentValid())
                    DisplayIndex++;
            }
            Container->SetChildIndex(Child->DisplayObject.ToSharedRef(), DisplayIndex);
        }
        else if (ChildrenRenderOrder == EChildrenRenderOrder::Descent)
        {
            for (int32 i = cnt - 1; i > Index; i--)
            {
                UGObject* Obj = Children[i];
                if (Obj->DisplayObject->IsParentValid())
                    DisplayIndex++;
            }
            Container->SetChildIndex(Child->DisplayObject.ToSharedRef(), DisplayIndex);
        }
        else
            BuildNativeDisplayList();

        SetBoundsChangedFlag();
    }

    return Index;
}

void UGComponent::SwapChildren(UGObject* Child1, UGObject* Child2)
{
    verifyf(Child1 != nullptr, TEXT("Argument1 must be non-nil"));
    verifyf(Child2 != nullptr, TEXT("Argument2 must be non-nil"));

    int32 Index1 = Children.Find(Child1);
    int32 Index2 = Children.Find(Child2);

    verifyf(Index1 != -1, TEXT("Not a child of this container"));
    verifyf(Index2 != -1, TEXT("Not a child of this container"));

    SwapChildrenAt(Index1, Index2);
}

void UGComponent::SwapChildrenAt(int32 Index1, int32 Index2)
{
    UGObject* Child1 = Children[Index1];
    UGObject* Child2 = Children[Index2];

    SetChildIndex(Child1, Index2);
    SetChildIndex(Child2, Index1);
}

int32 UGComponent::NumChildren() const
{
    return Children.Num();
}

bool UGComponent::IsAncestorOf(const UGObject* Obj) const
{
    if (Obj == nullptr)
        return false;

    UGComponent* Com = Obj->Parent.Get();
    while (Com != nullptr)
    {
        if (Com == this)
            return true;

        Com = Com->Parent.Get();
    }
    return false;
}

bool UGComponent::IsChildInView(UGObject* Child) const
{
    if (ScrollPane != nullptr)
    {
        return ScrollPane->IsChildInView(Child);
    }
    else if (DisplayObject->GetClipping() != EWidgetClipping::Inherit)
    {
        return Child->GetX() + Child->GetWidth() >= 0 && Child->GetX() <= GetWidth() && Child->GetY() + Child->GetHeight() >= 0 && Child->GetY() <= GetHeight();
    }
    else
        return true;
}

int32 UGComponent::GetFirstChildInView() const
{
    int32 i = 0;
    for (auto& Obj : Children)
    {

        if (IsChildInView(Obj))
            return i;
        i++;
    }
    return -1;
}

UGController* UGComponent::GetController(const FString& ControllerName) const
{
    for (const auto& Controller : Controllers)
    {
        if (Controller->Name.Compare(ControllerName) == 0)
            return Controller;
    }

    return nullptr;
}

void UGComponent::AddController(UGController* Controller)
{
    verifyf(Controller != nullptr, TEXT("Argument must be non-nil"));

    Controllers.Add(Controller);
}

UGController* UGComponent::GetControllerAt(int32 Index) const
{
    verifyf(Index >= 0 && Index < Controllers.Num(), TEXT("Invalid controller index"));

    return Controllers[Index];
}

void UGComponent::RemoveController(UGController* Controller)
{
    verifyf(Controller != nullptr, TEXT("Argument must be non-nil"));

    int32 Index = Controllers.Find(Controller);
    verifyf(Index != -1, TEXT("controller not exists"));

    ApplyController(Controller);
    Controllers.RemoveAt(Index);
}

void UGComponent::ApplyController(UGController* Controller)
{
    ApplyingController = Controller;

    for (int32 i = 0; i < Children.Num(); i++)
        Children[i]->HandleControllerChanged(Controller);

    ApplyingController = nullptr;

    Controller->RunActions();
}

void UGComponent::ApplyAllControllers()
{
    for (const auto& Controller : Controllers)
        ApplyController(Controller);
}

UTransition* UGComponent::GetTransition(const FString& TransitionName) const
{
    for (const auto& Transition : Transitions)
    {
        if (Transition->Name.Compare(TransitionName) == 0)
            return Transition;
    }

    return nullptr;
}

UTransition* UGComponent::GetTransitionAt(int32 Index) const
{
    verifyf(Index >= 0 && Index < Transitions.Num(), TEXT("Invalid transition index"));

    return Transitions[Index];
}

void UGComponent::AdjustRadioGroupDepth(UGObject* Obj, UGController* Controller)
{
    int32 cnt = Children.Num();
    int32 i;
    UGObject* Child;
    int32 myIndex = -1, maxIndex = -1;
    for (i = 0; i < cnt; i++)
    {
        Child = Children[i];
        if (Child == Obj)
        {
            myIndex = i;
        }
        else if (Child->IsA<UGButton>() && ((UGButton*)Child)->GetRelatedController() == Controller)
        {
            if (i > maxIndex)
                maxIndex = i;
        }
    }
    if (myIndex < maxIndex)
    {
        if (ApplyingController != nullptr)
            Children[maxIndex]->HandleControllerChanged(ApplyingController);
        SwapChildrenAt(myIndex, maxIndex);
    }
}

bool UGComponent::IsOpaque() const
{
    return DisplayObject->IsOpaque();
}

void UGComponent::SetOpaque(bool bInOpaque)
{
    DisplayObject->SetOpaque(bInOpaque);
}

void UGComponent::SetMargin(const FMargin& InMargin)
{
    Margin = InMargin;
}

void UGComponent::SetChildrenRenderOrder(EChildrenRenderOrder InRenderOrder)
{
    if (ChildrenRenderOrder != InRenderOrder)
    {
        ChildrenRenderOrder = InRenderOrder;
        BuildNativeDisplayList();
    }
}

void UGComponent::SetApexIndex(int32 InApexIndex)
{
    if (ApexIndex != InApexIndex)
    {
        ApexIndex = InApexIndex;

        if (ChildrenRenderOrder == EChildrenRenderOrder::Arch)
            BuildNativeDisplayList();
    }
}

float UGComponent::GetViewWidth() const
{
    if (ScrollPane != nullptr)
        return ScrollPane->GetViewSize().X;
    else
        return Size.X - Margin.Left - Margin.Right;
}

void UGComponent::SetViewWidth(float InViewWidth)
{
    if (ScrollPane != nullptr)
        ScrollPane->SetViewWidth(InViewWidth);
    else
        SetWidth(InViewWidth + Margin.Left + Margin.Right);
}

float UGComponent::GetViewHeight() const
{
    if (ScrollPane != nullptr)
        return ScrollPane->GetViewSize().Y;
    else
        return Size.Y - Margin.Top - Margin.Bottom;
}

void UGComponent::SetViewHeight(float InViewHeight)
{
    if (ScrollPane != nullptr)
        ScrollPane->SetViewHeight(InViewHeight);
    else
        SetHeight(InViewHeight + Margin.Top + Margin.Bottom);
}

void UGComponent::SetHitArea(const TSharedPtr<IHitTest>& InHitArea)
{
    HitArea = InHitArea;
    DisplayObject->UpdateVisibilityFlags();
}

void UGComponent::SetBoundsChangedFlag()
{
    if (bBoundsChanged)
        return;

    if (ScrollPane == nullptr && !bTrackBounds)
        return;

    bBoundsChanged = true;

    GetApp()->DelayCall(UpdateBoundsTimerHandle, this, &UGComponent::EnsureBoundsCorrect);
}

void UGComponent::EnsureBoundsCorrect()
{
    if (bBoundsChanged)
        UpdateBounds();
}

void UGComponent::UpdateBounds()
{
    float ax, ay, aw, ah;
    if (Children.Num() > 0)
    {
        ax = FLT_MAX;
        ay = FLT_MAX;
        float ar = -FLT_MAX, ab = -FLT_MAX;
        float tmp;

        int32 cnt = Children.Num();
        for (int32 i = 0; i < cnt; ++i)
        {
            UGObject* child = Children[i];
            tmp = child->GetX();
            if (tmp < ax)
                ax = tmp;
            tmp = child->GetY();
            if (tmp < ay)
                ay = tmp;
            tmp = child->GetX() + child->GetWidth();
            if (tmp > ar)
                ar = tmp;
            tmp = child->GetY() + child->GetHeight();
            if (tmp > ab)
                ab = tmp;
        }
        aw = ar - ax;
        ah = ab - ay;
    }
    else
    {
        ax = 0;
        ay = 0;
        aw = 0;
        ah = 0;
    }
    SetBounds(ax, ay, aw, ah);
}

void UGComponent::SetBounds(float ax, float ay, float aw, float ah)
{
    bBoundsChanged = false;
    if (ScrollPane != nullptr)
        ScrollPane->SetContentSize(FVector2D(FMath::CeilToFloat(ax + aw), FMath::CeilToFloat(ay + ah)));
}

void UGComponent::ChildStateChanged(UGObject* Child)
{
    if (bBuildingDisplayList)
        return;

    int32 cnt = Children.Num();
    if (Cast<UGGroup>(Child) != nullptr)
    {
        for (int32 i = 0; i < cnt; ++i)
        {
            UGObject* Obj = Children[i];
            if (Obj->GetGroup() == Child)
                ChildStateChanged(Obj);
        }
    }

    if (Child->InternalVisible())
    {
        if (!Child->DisplayObject->IsParentValid())
        {
            if (ChildrenRenderOrder == EChildrenRenderOrder::Ascent)
            {
                int32 index = 0;
                for (int32 i = 0; i < cnt; i++)
                {
                    UGObject* Obj = Children[i];
                    if (Obj == Child)
                        break;

                    if (Obj->DisplayObject->IsParentValid())
                        index++;
                }

                Container->AddChildAt(Child->DisplayObject.ToSharedRef(), index);
            }
            else if (ChildrenRenderOrder == EChildrenRenderOrder::Descent)
            {
                int32 index = 0;
                for (int32 i = cnt - 1; i >= 0; i--)
                {
                    UGObject* Obj = Children[i];
                    if (Obj == Child)
                        break;

                    if (Obj->DisplayObject->IsParentValid())
                        index++;
                }

                Container->AddChildAt(Child->DisplayObject.ToSharedRef(), index);
            }
            else
            {
                BuildNativeDisplayList();
            }
        }
    }
    else
    {
        if (Child->DisplayObject->IsParentValid())
        {
            Container->RemoveChild(Child->DisplayObject.ToSharedRef());
            if (ChildrenRenderOrder == EChildrenRenderOrder::Arch)
            {
                BuildNativeDisplayList();
            }
        }
    }
}

void UGComponent::ChildSortingOrderChanged(UGObject* Child, int32 OldValue, int32 NewValue)
{
    if (NewValue == 0)
    {
        SortingChildCount--;
        SetChildIndex(Child, Children.Num());
    }
    else
    {
        if (OldValue == 0)
            SortingChildCount++;

        int32 OldIndex = Children.Find(Child);
        int32 Index = GetInsertPosForSortingChild(Child);
        if (OldIndex < Index)
            MoveChild(Child, OldIndex, Index - 1);
        else
            MoveChild(Child, OldIndex, Index);
    }
}

void UGComponent::BuildNativeDisplayList(bool bImmediatelly)
{
    if (!bImmediatelly)
    {
        GetApp()->DelayCall(BuildDisplayListTimerHandle, this, &UGComponent::BuildNativeDisplayList, true);
        return;
    }

    int32 cnt = Children.Num();
    if (cnt == 0)
        return;

    switch (ChildrenRenderOrder)
    {
    case EChildrenRenderOrder::Ascent:
    {
        for (int32 i = 0; i < cnt; i++)
        {
            UGObject* Child = Children[i];
            if (Child->InternalVisible())
                Container->AddChild(Child->DisplayObject.ToSharedRef());
        }
    }
    break;
    case EChildrenRenderOrder::Descent:
    {
        for (int32 i = 0; i < cnt; i++)
        {
            UGObject* Child = Children[i];
            if (Child->InternalVisible())
                Container->AddChild(Child->DisplayObject.ToSharedRef());
        }
    }
    break;

    case EChildrenRenderOrder::Arch:
    {
        int32 ai = FMath::Min(ApexIndex, cnt);
        for (int32 i = 0; i < ai; i++)
        {
            UGObject* Child = Children[i];
            if (Child->InternalVisible())
                Container->AddChild(Child->DisplayObject.ToSharedRef());
        }
        for (int32 i = cnt - 1; i >= ai; i--)
        {
            UGObject* Child = Children[i];
            if (Child->InternalVisible())
                Container->AddChild(Child->DisplayObject.ToSharedRef());
        }
    }
    break;

    default:
        break;
    }
}

FVector2D UGComponent::GetSnappingPosition(const FVector2D& InPoint)
{
    int32 cnt = Children.Num();
    if (cnt == 0)
        return InPoint;

    EnsureBoundsCorrect();

    UGObject* Obj = nullptr;

    FVector2D ret = InPoint;

    int32 i = 0;
    if (ret.Y != 0)
    {
        for (; i < cnt; i++)
        {
            Obj = Children[i];
            if (ret.Y < Obj->GetY())
            {
                if (i == 0)
                {
                    ret.Y = 0;
                    break;
                }
                else
                {
                    UGObject* prev = Children[i - 1];
                    if (ret.Y < prev->GetY() + prev->GetHeight() / 2) //top half part
                        ret.Y = prev->GetY();
                    else //bottom half part
                        ret.Y = Obj->GetY();
                    break;
                }
            }
        }

        if (i == cnt)
            ret.Y = Obj->GetY();
    }

    if (ret.X != 0)
    {
        if (i > 0)
            i--;
        for (; i < cnt; i++)
        {
            Obj = Children[i];
            if (ret.X < Obj->GetX())
            {
                if (i == 0)
                {
                    ret.X = 0;
                    break;
                }
                else
                {
                    UGObject* prev = Children[i - 1];
                    if (ret.X < prev->GetX() + prev->GetWidth() / 2) // top half part
                        ret.X = prev->GetX();
                    else //bottom half part
                        ret.X = Obj->GetX();
                    break;
                }
            }
        }
        if (i == cnt)
            ret.X = Obj->GetX();
    }

    return ret;
}

void UGComponent::SetupOverflow(EOverflowType InOverflow)
{
    if (InOverflow == EOverflowType::Hidden)
    {
        DisplayObject->SetClipping(EWidgetClipping::ClipToBoundsAlways);
        DisplayObject->SetCullingBoundsExtension(Margin);
    }

    Container->SetPosition(Margin.GetTopLeft());
}

void UGComponent::SetupScroll(FByteBuffer* Buffer)
{
    ScrollPane = NewObject<UScrollPane>(this);
    ScrollPane->Setup(Buffer);
}

void UGComponent::HandleSizeChanged()
{
    UGObject::HandleSizeChanged();

    if (ScrollPane != nullptr)
        ScrollPane->OnOwnerSizeChanged();
    else
        Container->SetPosition(FVector2D(Margin.Left, Margin.Top));

    if (DisplayObject->GetClipping() != EWidgetClipping::Inherit)
        DisplayObject->SetCullingBoundsExtension(Margin);
}

void UGComponent::HandleGrayedChanged()
{
    UGObject::HandleGrayedChanged();

    UGController* Controller = GetController("grayed");
    if (Controller != nullptr)
        Controller->SetSelectedIndex(IsGrayed() ? 1 : 0);
    else
    {
        for (auto& Child : Children)
            Child->HandleGrayedChanged();
    }
}

void UGComponent::HandleControllerChanged(UGController* Controller)
{
    UGObject::HandleControllerChanged(Controller);

    if (ScrollPane != nullptr)
        ScrollPane->HandleControllerChanged(Controller);
}

void UGComponent::OnAddedToStageHandler(UEventContext* Context)
{
    for (auto& Transition : Transitions)
        Transition->OnOwnerAddedToStage();
}

void UGComponent::OnRemovedFromStageHandler(UEventContext* Context)
{
    for (auto& Transition : Transitions)
        Transition->OnOwnerRemovedFromStage();
}

void UGComponent::ConstructFromResource()
{
    ConstructFromResource(nullptr, 0);
}

void UGComponent::ConstructFromResource(TArray<UGObject*>* ObjectPool, int32 PoolIndex)
{
    TSharedPtr<FPackageItem> ContentItem = PackageItem->GetBranch();

    if (!ContentItem->bTranslated)
    {
        ContentItem->bTranslated = true;
        FTranslationHelper::TranslateComponent(ContentItem);
    }

    FByteBuffer* Buffer = ContentItem->RawData.Get();
    Buffer->Seek(0, 0);

    bUnderConstruct = true;

    SourceSize.X = Buffer->ReadInt();
    SourceSize.Y = Buffer->ReadInt();
    InitSize = SourceSize;

    SetSize(SourceSize);

    if (Buffer->ReadBool())
    {
        MinSize.X = Buffer->ReadInt();
        MaxSize.X = Buffer->ReadInt();
        MinSize.Y = Buffer->ReadInt();
        MaxSize.Y = Buffer->ReadInt();
    }

    if (Buffer->ReadBool())
    {
        float f1 = Buffer->ReadFloat();
        float f2 = Buffer->ReadFloat();
        SetPivot(FVector2D(f1, f2), Buffer->ReadBool());
    }

    if (Buffer->ReadBool())
    {
        Margin.Top = Buffer->ReadInt();
        Margin.Bottom = Buffer->ReadInt();
        Margin.Left = Buffer->ReadInt();
        Margin.Right = Buffer->ReadInt();
    }

    EOverflowType overflow = (EOverflowType)Buffer->ReadByte();
    if (overflow == EOverflowType::Scroll)
    {
        int32 savedPos = Buffer->GetPos();
        Buffer->Seek(0, 7);
        SetupScroll(Buffer);
        Buffer->SetPos(savedPos);
    }
    else
        SetupOverflow(overflow);

    if (Buffer->ReadBool()) //clip soft
        Buffer->Skip(8);

    bBuildingDisplayList = true;

    Buffer->Seek(0, 1);

    int32 controllerCount = Buffer->ReadShort();
    for (int32 i = 0; i < controllerCount; i++)
    {
        int32 nextPos = Buffer->ReadShort();
        nextPos += Buffer->GetPos();

        UGController* Controller = NewObject<UGController>(this);
        Controllers.Add(Controller);
        Controller->Setup(Buffer);

        Buffer->SetPos(nextPos);
    }

    Buffer->Seek(0, 2);

    UGObject* Child;
    int32 childCount = Buffer->ReadShort();
    for (int32 i = 0; i < childCount; i++)
    {
        int32 dataLen = Buffer->ReadShort();
        int32 curPos = Buffer->GetPos();

        if (ObjectPool != nullptr)
            Child = (*ObjectPool)[PoolIndex + i];
        else
        {
            Buffer->Seek(curPos, 0);

            EObjectType type = (EObjectType)Buffer->ReadByte();
            const FString& src = Buffer->ReadS();
            const FString& pkgId = Buffer->ReadS();

            TSharedPtr<FPackageItem> pii;
            if (!src.IsEmpty())
            {
                UUIPackage* pkg;
                if (!pkgId.IsEmpty())
                    pkg = UUIPackage::GetPackageByID(pkgId);
                else
                    pkg = ContentItem->Owner;

                if (pkg != nullptr)
                    pii = pkg->GetItem(src);
            }

            if (pii.IsValid())
            {
                Child = FUIObjectFactory::NewObject(pii, this);
                Child->ConstructFromResource();
            }
            else
                Child = FUIObjectFactory::NewObject(type, this);
        }

        Child->bUnderConstruct = true;
        Child->SetupBeforeAdd(Buffer, curPos);
        Child->Parent = this;
        Children.Add(Child);

        Buffer->SetPos(curPos + dataLen);
    }

    Buffer->Seek(0, 3);
    Relations->Setup(Buffer, true);

    Buffer->Seek(0, 2);
    Buffer->Skip(2);

    for (int32 i = 0; i < childCount; i++)
    {
        int32 nextPos = Buffer->ReadShort();
        nextPos += Buffer->GetPos();

        Buffer->Seek(Buffer->GetPos(), 3);
        Children[i]->GetRelations()->Setup(Buffer, false);

        Buffer->SetPos(nextPos);
    }

    Buffer->Seek(0, 2);
    Buffer->Skip(2);

    for (int32 i = 0; i < childCount; i++)
    {
        int32 nextPos = Buffer->ReadShort();
        nextPos += Buffer->GetPos();

        Child = Children[i];
        Child->SetupAfterAdd(Buffer, Buffer->GetPos());
        Child->bUnderConstruct = false;

        Buffer->SetPos(nextPos);
    }

    Buffer->Seek(0, 4);

    Buffer->Skip(2); //customData
    SetOpaque(Buffer->ReadBool());
    int32 maskId = Buffer->ReadShort();
    if (maskId != -1)
    {
        bool inverted = Buffer->ReadBool();
        //setMask(getChildAt(maskId)->displayObject(), inverted);
    }

    const FString& hitTestId = Buffer->ReadS();
    int32 i1 = Buffer->ReadInt();
    int32 i2 = Buffer->ReadInt();
    if (!hitTestId.IsEmpty())
    {
        TSharedPtr<FPackageItem> pii = ContentItem->Owner->GetItem(hitTestId);
        if (pii.IsValid() && pii->PixelHitTestData.IsValid())
            SetHitArea(MakeShareable(new FPixelHitTest(pii->PixelHitTestData, i1, i2)));
    }
    else if (i1 != 0 && i2 != -1)
    {
        SetHitArea(MakeShareable(new FChildHitTest(GetChildAt(i2))));
    }

    if (Buffer->Version >= 5)
    {
        const FString& enterSound = Buffer->ReadS();
        if (!enterSound.IsEmpty()) {
            On(FUIEvents::AddedToStage).AddLambda([this, enterSound](UEventContext*) {
                GetApp()->PlaySound(enterSound, 1);
            });
        }


        const FString& leaveSound = Buffer->ReadS();
        if (!leaveSound.IsEmpty()) {
            On(FUIEvents::RemovedFromStage).AddLambda([this, leaveSound](UEventContext*) {
                GetApp()->PlaySound(leaveSound, 1);
            });
        }
    }

    Buffer->Seek(0, 5);

    int32 transitionCount = Buffer->ReadShort();
    for (int32 i = 0; i < transitionCount; i++)
    {
        int32 nextPos = Buffer->ReadShort();
        nextPos += Buffer->GetPos();

        UTransition* Transition = NewObject<UTransition>(this);
        Transitions.Add(Transition);
        Transition->Setup(Buffer);

        Buffer->SetPos(nextPos);
    }

    if (Transitions.Num() > 0) {
        On(FUIEvents::AddedToStage).AddUObject(this, &UGComponent::OnAddedToStageHandler);
        On(FUIEvents::RemovedFromStage).AddUObject(this, &UGComponent::OnRemovedFromStageHandler);
    }

    ApplyAllControllers();

    bBuildingDisplayList = false;
    bUnderConstruct = false;

    BuildNativeDisplayList();
    SetBoundsChangedFlag();

    if (ContentItem->ObjectType != EObjectType::Component)
        ConstructExtension(Buffer);

    OnConstruct();
}

void UGComponent::ConstructExtension(FByteBuffer* Buffer)
{
}

void UGComponent::OnConstruct()
{
    K2_OnConstruct();
}

void UGComponent::SetupAfterAdd(FByteBuffer* Buffer, int32 BeginPos)
{
    UGObject::SetupAfterAdd(Buffer, BeginPos);

    Buffer->Seek(BeginPos, 4);

    int32 pageController = Buffer->ReadShort();
    if (pageController != -1 && ScrollPane != nullptr && ScrollPane->bPageMode)
        ScrollPane->PageController = Parent->GetControllerAt(pageController);

    int32 cnt = Buffer->ReadShort();
    for (int32 i = 0; i < cnt; i++)
    {
        UGController* Controller = GetController(Buffer->ReadS());
        const FString& PageID = Buffer->ReadS();
        if (Controller != nullptr)
            Controller->SetSelectedPageID(PageID);
    }

    if (Buffer->Version >= 2)
    {
        cnt = Buffer->ReadShort();
        for (int32 i = 0; i < cnt; i++)
        {
            FString Target = Buffer->ReadS();
            EObjectPropID PropID = (EObjectPropID)Buffer->ReadShort();
            FString Value = Buffer->ReadS();
            UGObject* Obj = GetChildByPath(Target);
            if (Obj != nullptr)
                Obj->SetProp(PropID, FNVariant(Value));
        }
    }
}

