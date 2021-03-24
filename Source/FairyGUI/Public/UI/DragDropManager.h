#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Event/EventContext.h"
#include "UI/GLoader.h"
#include "DragDropManager.generated.h"

UCLASS(BlueprintType)
class UDragDropManager : public UObject
{
    GENERATED_BODY()

public:
    UDragDropManager();
    virtual ~UDragDropManager();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    UGLoader* GetAgent() const { return Agent; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsDragging() const { return Agent->GetParent() != nullptr; }

    UFUNCTION(BlueprintCallable, Category = "FairyGUI", meta = (AutoCreateRefTerm="InSourceData"))
    void StartDrag(const FString& InIcon, const FNVariant& InUserData, int32 InUserIndex = -1, int32 InPointerIndex = -1);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void Cancel();

    void CreateAgent();
private:
    void DelayStartDrag(int32 InUserIndex, int32 InPointerIndex);
    void OnDragEnd(UEventContext* Context);

    UPROPERTY(Transient)
    UGLoader* Agent;
    FNVariant UserData;
};
