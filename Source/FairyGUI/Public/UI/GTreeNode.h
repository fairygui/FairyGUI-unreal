#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Utils/NVariant.h"
#include "GTreeNode.generated.h"

class UGComponent;
class UGTree;

UCLASS(BlueprintType, Blueprintable)
class FAIRYGUI_API UGTreeNode : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "FairyGUI", meta = (DisplayName = "Create Tree Node"))
    static UGTreeNode* CreateNode(bool bIsFolder = false, const FString& ResourceURL = "");

    UGTreeNode();
    virtual ~UGTreeNode();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGTreeNode* GetParent() const { return Parent.Get(); }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetParent(UGTreeNode* InParent);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGTree* GetTree() const { return Tree.Get(); }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGComponent* GetCell() const { return Cell; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsExpanded() const { return bExpanded; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetExpaned(bool bInExpanded);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsFolder() const { return bIsFolder; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    const FString& GetText() const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetText(const FString& InText);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    const FString& GetIcon() const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetIcon(const FString& InIcon);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGTreeNode* AddChild(UGTreeNode* Child);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGTreeNode* AddChildAt(UGTreeNode* Child, int32 Index);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void RemoveChild(UGTreeNode* Child);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void RemoveChildAt(int32 Index);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void RemoveChildren(int32 BeginIndex = 0, int32 EndIndex = -1);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGTreeNode* GetChildAt(int32 Index) const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGTreeNode* GetPrevSibling() const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGTreeNode* GetNextSibling() const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 GetChildIndex(const UGTreeNode* Child) const;

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetChildIndex(UGTreeNode* Child, int32 Index);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SwapChildren(UGTreeNode* Child, UGTreeNode* Child2);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SwapChildrenAt(int32 Index1, int32 Index2);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 NumChildren() const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FairyGUI")
    FNVariant UserData;

private:
    int32 MoveChild(UGTreeNode* Child, int32 OldIndex, int32 Index);
    void SetTree(UGTree* InTree);

    TWeakObjectPtr<UGTree> Tree;
    TWeakObjectPtr<UGTreeNode> Parent;
    UPROPERTY()
    UGComponent* Cell;
    UPROPERTY()
    TArray<UGTreeNode*> Children;

    int32 Level;
    bool bExpanded;
    bool bIsFolder;
    FString ResourceURL;

    friend class UGTree;
};