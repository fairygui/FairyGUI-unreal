#include "UI/GController.h"
#include "UI/GComponent.h"
#include "UI/UIPackage.h"
#include "Utils/ByteBuffer.h"

UGController::UGController() :
    SelectedIndex(-1),
    PreviousIndex(-1)
{
}

UGController::~UGController()
{
}

void UGController::SetSelectedIndex(int32 Index)
{
    SetSelectedIndex(Index, true);
}

void UGController::SetSelectedIndex(int32 Index, bool bTriggerEvent)
{
    if (SelectedIndex != Index)
    {
        verifyf(Index < PageIDs.Num(), TEXT("Invalid selected index"));

        bChanging = true;

        PreviousIndex = SelectedIndex;
        SelectedIndex = Index;
        Cast<UGComponent>(GetOuter())->ApplyController(this);

        if (bTriggerEvent)
            OnChangedEvent.Broadcast(this);

        bChanging = false;
    }
}

const FString& UGController::GetSelectedPage() const
{
    if (SelectedIndex == -1)
        return G_EMPTY_STRING;
    else
        return PageNames[SelectedIndex];
}

void UGController::SetSelectedPage(const FString& PageName)
{
    SetSelectedPage(PageName, true);
}

void UGController::SetSelectedPage(const FString& PageName, bool bTriggerEvent)
{
    int32 i = PageNames.Find(PageName);
    if (i == INDEX_NONE)
        i = 0;
    SetSelectedIndex(i, bTriggerEvent);
}

const FString& UGController::GetSelectedPageID() const
{
    if (SelectedIndex == -1)
        return G_EMPTY_STRING;
    else
        return PageIDs[SelectedIndex];
}

void UGController::SetSelectedPageID(const FString& PageID, bool bTriggerEvent)
{
    int32 i = PageIDs.Find(PageID);
    if (i != INDEX_NONE)
        SetSelectedIndex(i, bTriggerEvent);
}

const FString& UGController::GetPreviousPage() const
{
    if (PreviousIndex == -1)
        return G_EMPTY_STRING;
    else
        return PageNames[PreviousIndex];
}

const FString& UGController::GetPreviousPageID() const
{
    if (PreviousIndex == -1)
        return G_EMPTY_STRING;
    else
        return PageIDs[PreviousIndex];
}

int32 UGController::GetPageCount() const
{
    return PageIDs.Num();
}

bool UGController::HasPage(const FString& PageName) const
{
    return PageNames.Find(PageName) != INDEX_NONE;
}

int32 UGController::GetPageIndexByID(const FString& PageID) const
{
    return PageIDs.Find(PageID) != INDEX_NONE;
}

const FString& UGController::GetPageNameByID(const FString& PageID) const
{
    int32 i = PageIDs.Find(PageID);
    if (i != INDEX_NONE)
        return PageNames[i];
    else
        return G_EMPTY_STRING;
}

const FString& UGController::GetPageID(int32 Index) const
{
    return PageIDs[Index];
}

void UGController::SetOppositePageID(const FString& PageID)
{
    int32 i = PageIDs.Find(PageID);
    if (i > 0)
        SetSelectedIndex(0);
    else if (PageIDs.Num() > 1)
        SetSelectedIndex(1);
}

void UGController::RunActions()
{
    if (Actions.Num() == 0)
        return;

    for (auto& it : Actions)
        it.Run(this, GetPreviousPageID(), GetSelectedPageID());
}

void UGController::Setup(FByteBuffer* Buffer)
{
    int32 BeginPos = Buffer->GetPos();
    Buffer->Seek(BeginPos, 0);

    Name = Buffer->ReadS();
    bAutoRadioGroupDepth = Buffer->ReadBool();

    Buffer->Seek(BeginPos, 1);

    int32 cnt = Buffer->ReadShort();
    PageIDs.SetNum(cnt);
    PageNames.SetNum(cnt);
    for (int32 i = 0; i < cnt; i++)
    {
        PageIDs[i] = Buffer->ReadS();
        PageNames[i] = Buffer->ReadS();
    }

    int32 HomePageIndex = 0;
    if (Buffer->Version >= 2)
    {
        int32 HomePageType = Buffer->ReadByte();
        switch (HomePageType)
        {
        case 1:
            HomePageIndex = Buffer->ReadShort();
            break;

        case 2:
            HomePageIndex = PageNames.Find(UUIPackage::GetBranch());
            if (HomePageIndex == INDEX_NONE)
                HomePageIndex = 0;
            break;

        case 3:
            HomePageIndex = PageNames.Find(UUIPackage::GetVar(Buffer->ReadS()));
            if (HomePageIndex == INDEX_NONE)
                HomePageIndex = 0;
            break;
        }
    }

    Buffer->Seek(BeginPos, 2);

    cnt = Buffer->ReadShort();
    if (cnt > 0)
    {
        for (int32 i = 0; i < cnt; i++)
        {
            int32 nextPos = Buffer->ReadShort();
            nextPos += Buffer->GetPos();

            FControllerAction* Action = FControllerAction::CreateAction(Buffer->ReadByte());
            Action->Setup(Buffer);
            Actions.Add(Action);

            Buffer->SetPos(nextPos);
        }
    }

    if (PageIDs.Num() > 0)
        SelectedIndex = HomePageIndex;
    else
        SelectedIndex = -1;
}