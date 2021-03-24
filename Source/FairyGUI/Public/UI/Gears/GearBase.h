#pragma once

#include "CoreMinimal.h"
#include "Tween/GTween.h"

class UGObject;
class UGController;
class FByteBuffer;

class FAIRYGUI_API FGearTweenConfig
{
public:
    FGearTweenConfig();

    bool bTween;
    EEaseType EaseType;
    float Duration;
    float Delay;

    uint32 DisplayLockToken;
    FTweenerHandle Handle;
};

class FGearBase
{
public:
    enum class EType
    {
        Display,
        XY,
        Size,
        Look,
        Color,
        Animation,
        Text,
        Icon,
        Display2,
        FontSize
    };

    FGearBase(UGObject* InOwner);
    virtual ~FGearBase();

    EType GetType() const { return Type; }

    UGController* GetController() const { return Controller; }
    void SetController(UGController* InController);

    FGearTweenConfig& GetTweenConfig();

    virtual void UpdateFromRelations(const FVector2D& Delta);
    virtual void Apply();
    virtual void UpdateState();

    void Setup(FByteBuffer* Buffer);

    static TSharedPtr<FGearBase> Create(UGObject* InOwner, EType InType);
    static bool bDisableAllTweenEffect;

protected:
    virtual void AddStatus(const FString& PageID, FByteBuffer* Buffer);
    virtual void Init();

    EType Type;
    UGObject* Owner;
    UGController* Controller;
    TOptional<FGearTweenConfig> TweenConfig;
};