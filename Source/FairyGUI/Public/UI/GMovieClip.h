#pragma once

#include "GObject.h"
#include "Event/EventContext.h"
#include "GMovieClip.generated.h"

UCLASS(BlueprintType)
class FAIRYGUI_API UGMovieClip : public UGObject
{
    GENERATED_BODY()

public:
    UGMovieClip();
    virtual ~UGMovieClip();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    bool IsPlaying() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetPlaying(bool bInPlaying);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    int32 GetFrame() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetFrame(int32 InFrame);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    float GetTimeScale() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetTimeScale(float InTimeScale);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void Advance(float Time);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    EFlipType GetFlip() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetFlip(EFlipType InFlip);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    FColor GetColor() const;
    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    void SetColor(const FColor& InColor);

    //from start to end(-1 means ending) repeat times(0 means infinite loop) when all is over, stopping at endAt(-1 means same value of end)
    void SetPlaySettings(int32 InStart = 0, int32 InEnd = -1, int32 InTimes = 0, int32 InEndAt = -1, const FSimpleDelegate& InCompleteCallback = FSimpleDelegate());

    UFUNCTION(BlueprintCallable, Category = "FairyGUI", meta = (AutoCreateRefTerm = "InCompleteCallback"))
    void SetPlaySettings(const FSimpleDynDelegate& InCompleteCallback, int32 InStart = 0, int32 InEnd = -1, int32 InTimes = 0, int32 InEndAt = -1)
    {
        SetPlaySettings(InStart, InEnd, InTimes, InEndAt,
            InCompleteCallback.IsBound() ?
            FSimpleDelegate::CreateUFunction(const_cast<UObject*>(InCompleteCallback.GetUObject()), InCompleteCallback.GetFunctionName())
            : FSimpleDelegate());
    }

    virtual void ConstructFromResource() override;

    virtual FNVariant GetProp(EObjectPropID PropID) const override;
    virtual void SetProp(EObjectPropID PropID, const FNVariant& InValue) override;

protected:
    virtual void SetupBeforeAdd(FByteBuffer* Buffer, int32 BeginPos) override;

private:
    TSharedPtr<class SMovieClip> Content;
};