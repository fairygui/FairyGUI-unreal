#include "UI/GTreeNode.h"
#include "UI/GComponent.h"
#include "UI/GTree.h"

UGTreeNode* UGTreeNode::CreateNode(bool bIsFolder, const FString& ResourceURL)
{
    UGTreeNode* Node = NewObject<UGTreeNode>();
    Node->bIsFolder = bIsFolder;
    Node->ResourceURL = ResourceURL;

    return Node;
}

UGTreeNode::UGTreeNode()
{
}

UGTreeNode::~UGTreeNode()
{
}

void UGTreeNode::SetParent(UGTreeNode* InParent)
{
    verifyf(InParent == nullptr || InParent->IsFolder(), TEXT("Parent must be a folder node"));
    verifyf(InParent != this, TEXT("Parent must not be self"));

    if (InParent != nullptr)
        InParent->AddChild(this);
    else if (Parent.IsValid())
        Parent->RemoveChild(this);
}

void UGTreeNode::SetExpaned(bool bInExpanded)
{
    if (!bIsFolder)
        return;

    if (bExpanded != bInExpanded)
    {
        bExpanded = bInExpanded;
        if (Tree.IsValid())
        {
            if (bExpanded)
                Tree->AfterExpanded(this);
            else
                Tree->AfterCollapsed(this);
        }
    }
}

const FString& UGTreeNode::GetText() const
{
    if (Cell != nullptr)
        return Cell->GetText();
    else
        return G_EMPTY_STRING;
}

void UGTreeNode::SetText(const FString& InText)
{
    if (Cell != nullptr)
        return Cell->SetText(InText);
}

const FString& UGTreeNode::GetIcon() const
{
    if (Cell != nullptr)
        return Cell->GetIcon();
    else
        return G_EMPTY_STRING;
}

void UGTreeNode::SetIcon(const FString& InIcon)
{
    if (Cell != nullptr)
        return Cell->SetIcon(InIcon);
}

UGTreeNode* UGTreeNode::AddChild(UGTreeNode* Child)
{
    AddChildAt(Child, Children.Num());
    return Child;
}

UGTreeNode* UGTreeNode::AddChildAt(UGTreeNode* Child, int32 Index)
{
    verifyf(Child != nullptr, TEXT("Argument must be non-nil"));

    if (Child->Parent == this)
    {
        SetChildIndex(Child, Index);
    }
    else
    {
        if (Child->Parent.IsValid())
            Child->Parent->RemoveChild(Child);
        Child->Parent = this;

        int32 cnt = Children.Num();
        if (Index == cnt)
            Children.Add(Child);
        else
            Children.Insert(Child, Index);

        Child->Level = Level + 1;
        Child->SetTree(Tree.Get());
        if ((Tree.IsValid() && this == Tree->GetRootNode()) || (Cell != nullptr && Cell->GetParent() != nullptr && bExpanded))
            Tree->AfterInserted(Child);
    }
    return Child;
}

void UGTreeNode::RemoveChild(UGTreeNode* Child)
{
    verifyf(Child != nullptr, TEXT("Argument must be non-nil"));

    int32 ChildIndex = Children.Find(Child);
    if (ChildIndex != INDEX_NONE)
        RemoveChildAt(ChildIndex);
}

void UGTreeNode::RemoveChildAt(int32 Index)
{
    verifyf(Index >= 0 && Index < Children.Num(), TEXT("Invalid child index"));

    UGTreeNode* Child = Children[Index];
    Child->Parent = nullptr;

    if (Tree.IsValid())
    {
        Child->SetTree(nullptr);
        Tree->AfterRemoved(Child);
    }

    Children.RemoveAt(Index);
}

void UGTreeNode::RemoveChildren(int32 BeginIndex, int32 EndIndex)
{
    if (EndIndex < 0 || EndIndex >= Children.Num())
        EndIndex = Children.Num() - 1;

    for (int32 i = BeginIndex; i <= EndIndex; ++i)
        RemoveChildAt(BeginIndex);
}

UGTreeNode* UGTreeNode::GetChildAt(int32 Index) const
{
    verifyf(Index >= 0 && Index < Children.Num(), TEXT("Invalid child index"));

    return Children[Index];
}

UGTreeNode* UGTreeNode::GetPrevSibling() const
{
    if (!Parent.IsValid())
        return nullptr;

    int32 i = Parent->Children.IndexOfByKey(this);
    if (i <= 0)
        return nullptr;

    return Parent->Children[i - 1];
}

UGTreeNode* UGTreeNode::GetNextSibling() const
{
    if (!Parent.IsValid())
        return nullptr;

    int32 i = Parent->Children.IndexOfByKey(this);
    if (i < 0 || i >= Parent->Children.Num() - 1)
        return nullptr;

    return Parent->Children[i + 1];
}

int32 UGTreeNode::GetChildIndex(const UGTreeNode* Child) const
{
    verifyf(Child != nullptr, TEXT("Argument must be non-nil"));

    return Children.IndexOfByKey(Child);
}

void UGTreeNode::SetChildIndex(UGTreeNode* Child, int32 Index)
{
    verifyf(Child != nullptr, TEXT("Argument must be non-nil"));

    int32 OldIndex = Children.Find(Child);
    verifyf(OldIndex != -1, TEXT("Not a child of this container"));

    int32 cnt = Children.Num();
    if (Index < 0)
        Index = 0;
    else if (Index > cnt)
        Index = cnt;

    if (OldIndex == Index)
        return;

    Children.RemoveAt(OldIndex);
    Children.Insert(Child, Index);
    if ((Tree.IsValid() && this == Tree->RootNode) || (Cell != nullptr && Cell->GetParent() != nullptr && bExpanded))
        Tree->AfterMoved(Child);
}

void UGTreeNode::SwapChildren(UGTreeNode* Child1, UGTreeNode* Child2)
{
    int32 Index1 = Children.Find(Child1);
    int32 Index2 = Children.Find(Child2);

    verifyf(Index1 != -1, TEXT("Not a child of this container"));
    verifyf(Index2 != -1, TEXT("Not a child of this container"));

    SwapChildrenAt(Index1, Index2);
}

void UGTreeNode::SwapChildrenAt(int32 Index1, int32 Index2)
{
    UGTreeNode* Child1 = Children[Index1];
    UGTreeNode* Child2 = Children[Index2];

    SetChildIndex(Child1, Index2);
    SetChildIndex(Child2, Index1);
}

int32 UGTreeNode::NumChildren() const
{
    return Children.Num();
}

void UGTreeNode::SetTree(UGTree* InTree)
{
    Tree = InTree;
    if (Tree.IsValid() && Tree->OnTreeNodeWillExpand.IsBound() && bExpanded)
        Tree->OnTreeNodeWillExpand.Execute(this, true);

    if (bIsFolder)
    {
        for (auto& Child : Children)
        {
            Child->Level = Level + 1;
            Child->SetTree(InTree);
        }
    }
}