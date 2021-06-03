#include "UI/GTree.h"
#include "Utils/ByteBuffer.h"
#include "UI/GController.h"
#include "UI/GObjectPool.h"

UGTree::UGTree() :
    Indent(30)
{
    if (!HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
    {
        RootNode = UGTreeNode::CreateNode(true);
        RootNode->SetTree(this);
        RootNode->SetExpaned(true);
    }
}

UGTree::~UGTree()
{
}

UGTreeNode* UGTree::GetSelectedNode() const
{
    int32 i = GetSelectedIndex();
    if (i != -1)
        return GetChildAt(i)->TreeNode;
    else
        return nullptr;
}

void UGTree::GetSelectedNodes(TArray<UGTreeNode*>& Result) const
{
    TArray<int32> ids;
    GetSelection(ids);
    for (auto& it : ids)
    {
        UGTreeNode* Node = GetChildAt(it)->TreeNode;
        Result.Add(Node);
    }
}

void UGTree::SelectNode(UGTreeNode* Node, bool bScrollItToView)
{
    UGTreeNode* ParentNode = Node->Parent.Get();
    while (ParentNode != nullptr && ParentNode != RootNode)
    {
        ParentNode->SetExpaned(true);
        ParentNode = ParentNode->GetParent();
    }
    if (Node->Cell != nullptr)
        AddSelection(GetChildIndex(Node->Cell), bScrollItToView);
}

void UGTree::UnselectNode(UGTreeNode* Node)
{
    if (Node->Cell != nullptr)
        RemoveSelection(GetChildIndex(Node->Cell));
}

void UGTree::ExpandAll(UGTreeNode* FolderNode)
{
    FolderNode->SetExpaned(true);
    for (auto& it : FolderNode->Children)
    {
        if (it->IsFolder())
            ExpandAll(it);
    }
}

void UGTree::CollapseAll(UGTreeNode* FolderNode)
{
    if (FolderNode != RootNode)
        FolderNode->SetExpaned(false);
    for (auto& it : FolderNode->Children)
    {
        if (it->IsFolder())
            CollapseAll(it);
    }
}

void UGTree::CreateCell(UGTreeNode* Node)
{
    const FString& url = Node->ResourceURL.IsEmpty() ? GetDefaultItem() : Node->ResourceURL;
    UGComponent* Child = GetItemPool()->GetObject(url, this)->As<UGComponent>();
    verifyf(Child != nullptr, TEXT("Unable to create tree cell"));

    Child->TreeNode = Node;
    Node->Cell = Child;

    UGObject* IndentObj = Node->Cell->GetChild("indent");
    if (IndentObj != nullptr)
        IndentObj->SetWidth((Node->Level - 1) * Indent);

    UGController* cc;

    cc = Child->GetController("expanded");
    if (cc != nullptr)
    {
        cc->OnChanged().AddUObject(this, &UGTree::OnExpandedStateChanged);
        cc->SetSelectedIndex(Node->IsExpanded() ? 1 : 0);
    }

    cc = Child->GetController("leaf");
    if (cc != nullptr)
        cc->SetSelectedIndex(Node->IsFolder() ? 0 : 1);

    if (Node->IsFolder())
        Child->OnTouchBegin.AddUniqueDynamic(this, &UGTree::OnCellTouchBegin);

    TreeNodeRenderer.ExecuteIfBound(Node, Child);
}

void UGTree::AfterInserted(UGTreeNode* Node)
{
    if (Node->Cell == nullptr)
        CreateCell(Node);

    int32 Index = GetInsertIndexForNode(Node);
    AddChildAt(Node->Cell, Index);
    TreeNodeRenderer.ExecuteIfBound(Node, Node->Cell);

    if (Node->IsFolder() && Node->IsExpanded())
        CheckChildren(Node, Index);
}

int32 UGTree::GetInsertIndexForNode(UGTreeNode* Node)
{
    UGTreeNode* PrevNode = Node->GetPrevSibling();
    if (PrevNode == nullptr)
        PrevNode = Node->GetParent();
    int32 InsertIndex;
    if (PrevNode->Cell != nullptr)
        InsertIndex = GetChildIndex(PrevNode->Cell) + 1;
    else
        InsertIndex = 0;
    int32 myLevel = Node->Level;
    int32 cnt = NumChildren();
    for (int32 i = InsertIndex; i < cnt; i++)
    {
        UGTreeNode* TestNode = GetChildAt(i)->TreeNode;
        if (TestNode->Level <= myLevel)
            break;

        InsertIndex++;
    }

    return InsertIndex;
}

void UGTree::AfterRemoved(UGTreeNode* Node)
{
    RemoveNode(Node);
}

void UGTree::AfterExpanded(UGTreeNode* Node)
{
    if (Node == RootNode)
    {
        CheckChildren(RootNode, 0);
        return;
    }

    OnTreeNodeWillExpand.ExecuteIfBound(Node, true);

    if (Node->Cell == nullptr)
        return;

    TreeNodeRenderer.ExecuteIfBound(Node, Node->Cell);

    UGController* cc = Node->Cell->GetController("expanded");
    if (cc != nullptr)
        cc->SetSelectedIndex(1);

    if (Node->Cell->GetParent() != nullptr)
        CheckChildren(Node, GetChildIndex(Node->Cell));
}

void UGTree::AfterCollapsed(UGTreeNode* Node)
{
    if (Node == RootNode)
    {
        CheckChildren(RootNode, 0);
        return;
    }

    OnTreeNodeWillExpand.ExecuteIfBound(Node, false);

    if (Node->Cell == nullptr)
        return;

    TreeNodeRenderer.ExecuteIfBound(Node, Node->Cell);

    UGController* cc = Node->Cell->GetController("expanded");
    if (cc != nullptr)
        cc->SetSelectedIndex(0);

    if (Node->Cell->GetParent() != nullptr)
        HideFolderNode(Node);
}

void UGTree::AfterMoved(UGTreeNode* Node)
{
    int32 startIndex = GetChildIndex(Node->Cell);
    int32 endIndex;
    if (Node->IsFolder())
        endIndex = GetFolderEndIndex(startIndex, Node->Level);
    else
        endIndex = startIndex + 1;
    int32 insertIndex = GetInsertIndexForNode(Node);
    int32 cnt = endIndex - startIndex;

    if (insertIndex < startIndex)
    {
        for (int32 i = 0; i < cnt; i++)
        {
            UGObject* obj = GetChildAt(startIndex + i);
            SetChildIndex(obj, insertIndex + i);
        }
    }
    else
    {
        for (int32 i = 0; i < cnt; i++)
        {
            UGObject* obj = GetChildAt(startIndex);
            SetChildIndex(obj, insertIndex);
        }
    }
}

int32 UGTree::GetFolderEndIndex(int32 StartIndex, int32 Level)
{
    int32 cnt = NumChildren();
    for (int32 i = StartIndex + 1; i < cnt; i++)
    {
        UGTreeNode* Node = GetChildAt(i)->TreeNode;
        if (Node->Level <= Level)
            return i;
    }

    return cnt;
}

int32 UGTree::CheckChildren(UGTreeNode* FolderNode, int32 Index)
{
    int32 cnt = FolderNode->NumChildren();
    for (int32 i = 0; i < cnt; i++)
    {
        Index++;
        UGTreeNode* Node = FolderNode->GetChildAt(i);
        if (Node->Cell == nullptr)
            CreateCell(Node);

        if (Node->Cell->GetParent() == nullptr)
            AddChildAt(Node->Cell, Index);

        if (Node->IsFolder() && Node->IsExpanded())
            Index = CheckChildren(Node, Index);
    }

    return Index;
}

void UGTree::HideFolderNode(UGTreeNode* FolderNode)
{
    int32 cnt = FolderNode->NumChildren();
    for (int32 i = 0; i < cnt; i++)
    {
        UGTreeNode* Node = FolderNode->GetChildAt(i);
        if (Node->Cell != nullptr && Node->Cell->GetParent() != nullptr)
            RemoveChild(Node->Cell);

        if (Node->IsFolder() && Node->IsExpanded())
            HideFolderNode(Node);
    }
}

void UGTree::RemoveNode(UGTreeNode* Node)
{
    if (Node->Cell != nullptr)
    {
        if (Node->Cell->GetParent() != nullptr)
            RemoveChild(Node->Cell);
        GetItemPool()->ReturnObject(Node->Cell);
        Node->Cell->TreeNode = nullptr;
        Node->Cell = nullptr;
    }

    if (Node->IsFolder())
    {
        int32 cnt = Node->NumChildren();
        for (int32 i = 0; i < cnt; i++)
        {
            UGTreeNode* Node2 = Node->GetChildAt(i);
            RemoveNode(Node2);
        }
    }
}

void UGTree::OnCellTouchBegin(UEventContext* Context)
{
    UGTreeNode* Node = Context->GetSender()->TreeNode;
    bExpandedStatusInEvt = Node->IsExpanded();
}

void UGTree::OnExpandedStateChanged(UGController* Controller)
{
    UGTreeNode* Node = Cast<UGObject>(Controller->GetOuter())->TreeNode;
    Node->SetExpaned(Controller->GetSelectedIndex() == 1);
}

void UGTree::DispatchItemEvent(UGObject* Obj, UEventContext* Context)
{
    if (ClickToExpand != 0)
    {
        UGTreeNode* Node = Obj->TreeNode;
        if (Node != nullptr && bExpandedStatusInEvt == Node->IsExpanded())
        {
            if (ClickToExpand == 2)
            {
                if (Context->IsDoubleClick())
                    Node->SetExpaned(!Node->IsExpanded());
            }
            else
                Node->SetExpaned(!Node->IsExpanded());
        }
    }

    Super::DispatchItemEvent(Obj, Context);
}

void UGTree::SetupBeforeAdd(FByteBuffer* Buffer, int32 BeginPos)
{
    Super::SetupBeforeAdd(Buffer, BeginPos);

    Buffer->Seek(BeginPos, 9);

    Indent = Buffer->ReadInt();
    ClickToExpand = Buffer->ReadByte();
}

void UGTree::ReadItems(FByteBuffer* Buffer)
{
    int32 nextPos;
    FString str;
    bool bIsFolder;
    UGTreeNode* lastNode = nullptr;
    int32 level;
    int32 prevLevel = 0;

    int32 cnt = Buffer->ReadShort();
    for (int32 i = 0; i < cnt; i++)
    {
        nextPos = Buffer->ReadShort();
        nextPos += Buffer->GetPos();

        str = Buffer->ReadS();
        if (!str.IsEmpty())
        {
            str = GetDefaultItem();
            if (str.IsEmpty())
            {
                Buffer->SetPos(nextPos);
                continue;
            }
        }

        bIsFolder = Buffer->ReadBool();
        level = Buffer->ReadByte();

        UGTreeNode* node = UGTreeNode::CreateNode(bIsFolder, str);
        node->SetExpaned(true);
        if (i == 0)
            RootNode->AddChild(node);
        else
        {
            if (level > prevLevel)
                lastNode->AddChild(node);
            else if (level < prevLevel)
            {
                for (int32 j = level; j <= prevLevel; j++)
                    lastNode = lastNode->GetParent();
                lastNode->AddChild(node);
            }
            else
                lastNode->GetParent()->AddChild(node);
        }
        lastNode = node;
        prevLevel = level;

        SetupItem(Buffer, node->Cell);

        Buffer->SetPos(nextPos);
    }
}
