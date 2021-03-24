#pragma once

#include "GList.h"
#include "GTreeNode.h"
#include "GTree.generated.h"

DECLARE_DELEGATE_TwoParams(FTreeNodeRenderer, UGTreeNode*, UGComponent*);
DECLARE_DELEGATE_TwoParams(FOnTreeNodeWillExpand, UGTreeNode*, bool);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FDynTreeNodeRenderer, UGTreeNode*, Node, UGComponent*, Obj);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FDynOnTreeNodeWillExpand, UGTreeNode*, Node, bool, bToExpand);

UCLASS(BlueprintType)
class FAIRYGUI_API UGTree : public UGList
{
    GENERATED_BODY()

public:
    UGTree();
    virtual ~UGTree();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGTreeNode* GetRootNode() const { return RootNode; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGTreeNode* GetSelectedNode() const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void GetSelectedNodes(TArray<UGTreeNode*>& Result) const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SelectNode(UGTreeNode* Node, bool bScrollItToView = false);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void UnselectNode(UGTreeNode* Node);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void ExpandAll(UGTreeNode* Node);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void CollapseAll(UGTreeNode* Node);

    void SetTreeNodeRenderer(const FTreeNodeRenderer& InDelegate) { TreeNodeRenderer = InDelegate; }
    void SetOnTreeNodeWillExpand(const FOnTreeNodeWillExpand& InDelegate) { OnTreeNodeWillExpand = InDelegate; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetTreeNodeRenderer(const FDynTreeNodeRenderer& InDelegate)
    {
        if (InDelegate.IsBound())
            TreeNodeRenderer = FTreeNodeRenderer::CreateUFunction(const_cast<UObject*>(InDelegate.GetUObject()), InDelegate.GetFunctionName());
        else
            TreeNodeRenderer.Unbind();
    }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
     void SetOnTreeNodeWillExpand(const FDynOnTreeNodeWillExpand& InDelegate)
    {
        if (InDelegate.IsBound())
            OnTreeNodeWillExpand = FOnTreeNodeWillExpand::CreateUFunction(const_cast<UObject*>(InDelegate.GetUObject()), InDelegate.GetFunctionName());
        else
            OnTreeNodeWillExpand.Unbind();
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    int32 Indent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    int32 ClickToExpand;

protected:
    virtual void SetupBeforeAdd(FByteBuffer* Buffer, int32 BeginPos) override;
    virtual void ReadItems(FByteBuffer* Buffer) override;
    virtual void DispatchItemEvent(UGObject* Obj, UEventContext* Context) override;

private:
    void CreateCell(UGTreeNode* Node);
    void AfterInserted(UGTreeNode* Node);
    int32 GetInsertIndexForNode(UGTreeNode* Node);
    void AfterRemoved(UGTreeNode* Node);
    void AfterExpanded(UGTreeNode* Node);
    void AfterCollapsed(UGTreeNode* Node);
    void AfterMoved(UGTreeNode* Node);
    int32 CheckChildren(UGTreeNode* FolderNode, int32 Index);
    void HideFolderNode(UGTreeNode* FolderNode);
    void RemoveNode(UGTreeNode* Node);
    int32 GetFolderEndIndex(int32 StartIndex, int32 Level);

    UFUNCTION()
    void OnCellTouchBegin(UEventContext* Context);
    void OnExpandedStateChanged(UGController* Controller);

    UPROPERTY()
    UGTreeNode* RootNode;
    bool bExpandedStatusInEvt;
    FTreeNodeRenderer TreeNodeRenderer;
    FOnTreeNodeWillExpand OnTreeNodeWillExpand;

    friend class UGTreeNode;
};