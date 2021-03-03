#include "UI/DragDropManager.h"
#include "FairyApplication.h"

UDragDropManager* UDragDropManager::Get()
{
    return UFairyApplication::Get()->DragDropManager;
}

UDragDropManager::UDragDropManager()
{
    Agent = (UGLoader*)FUIObjectFactory::NewObject(EObjectType::Loader);
    Agent->Name = TEXT("DragDropAgent");
    Agent->SetTouchable(false);
    Agent->SetDraggable(true);
    Agent->SetSize(FVector2D(100, 100));
    Agent->SetPivot(FVector2D(.5f, .5f), true);
    Agent->SetAlign(EAlignType::Center);
    Agent->SetVerticalAlign(EVerticalAlignType::Middle);
    Agent->SetSortingOrder(INT_MAX);
    Agent->On(FUIEvents::DragEnd).AddUObject(this, &UDragDropManager::OnDragEnd);
}

UDragDropManager::~UDragDropManager()
{
}

void UDragDropManager::StartDrag(const FString& InIcon, const FNVariant& InUserData, int32 InUserIndex, int32 InPointerIndex)
{
    if (Agent->GetParent() != nullptr)
        return;

    UserData = InUserData;
    Agent->SetURL(InIcon);
    UGRoot::Get()->AddChild(Agent);
    FVector2D pt = UGRoot::Get()->GlobalToLocal(UFairyApplication::Get()->GetTouchPosition(InUserIndex, InPointerIndex));
    Agent->SetPosition(pt);
    UFairyApplication::Get()->CallAfterSlateTick(FSimpleDelegate::CreateUObject(this, &UDragDropManager::DelayStartDrag, InUserIndex, InPointerIndex));
}

void UDragDropManager::DelayStartDrag(int32 InUserIndex, int32 InPointerIndex)
{
    if (Agent->GetParent() != nullptr)
        Agent->StartDrag(InUserIndex, InPointerIndex);
}

void UDragDropManager::Cancel()
{
    if (Agent->GetParent() != nullptr)
    {
        Agent->StopDrag();
        Agent->RemoveFromParent();
        UserData.Reset();
    }
}

void UDragDropManager::OnDragEnd(UEventContext* Context)
{
    if (Agent->GetParent() == nullptr) //canceled
        return;

    Agent->RemoveFromParent();

    UGObject* obj = UFairyApplication::Get()->GetObjectUnderPoint(Context->GetPointerPosition());
    while (obj != nullptr)
    {
        if (obj->IsA<UGComponent>())
        {
            if (obj->HasEventListener(FUIEvents::Drop))
            {
                obj->DispatchEvent(FUIEvents::Drop, UserData);
                return;
            }
        }

        obj = obj->GetParent();
    }
}