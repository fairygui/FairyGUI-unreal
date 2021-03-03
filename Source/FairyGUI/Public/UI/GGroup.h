#pragma once

#include "GObject.h"
#include "GGroup.generated.h"

UCLASS(BlueprintType)
class FAIRYGUI_API UGGroup : public UGObject
{
    GENERATED_BODY()

public:
    UGGroup();
    virtual ~UGGroup();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    EGroupLayoutType GetLayout() { return Layout; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetLayout(EGroupLayoutType InLayout);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 GetColumnGap() { return ColumnGap; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetColumnGap(int32 InColumnGap);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 GetLineGap() { return LineGap; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetLineGap(int32 InLineGap);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsExcludeInvisibles() { return bExcludeInvisibles; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetExcludeInvisibles(bool bInFlag);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsAutoSizeDisabled() { return bAutoSizeDisabled; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetAutoSizeDisabled(bool bInFlag);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 GetMainGridIndex() { return MainGridIndex; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetMainGridIndex(int32 InIndex);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 GetMainGridMinSize() { return MainGridMinSize; }
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetMainGridMinSize(int32 InSize);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetBoundsChangedFlag(bool bPositionChangedOnly = false);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void EnsureBoundsCorrect();

    void MoveChildren(const FVector2D& Delta);
    void ResizeChildren(const FVector2D& Delta);

    uint8 Updating;

protected:
    virtual void SetupBeforeAdd(FByteBuffer* Buffer, int32 BeginPos) override;
    virtual void SetupAfterAdd(FByteBuffer* Buffer, int32 BeginPos) override;
    virtual void HandleAlphaChanged() override;
    virtual void HandleVisibleChanged() override;

private:
    void UpdateBounds();
    void HandleLayout();

    EGroupLayoutType Layout;
    int32 LineGap;
    int32 ColumnGap;
    bool bExcludeInvisibles;
    bool bAutoSizeDisabled;
    int32 MainGridIndex;
    int32 MainGridMinSize;

    bool bPercentReady;
    bool bBoundsChanged;
    int32 MainChildIndex;
    float TotalSize;
    int32 NumChildren;

    FTimerHandle UpdateBoundsTimerHandle;
};