#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Utils/NVariant.h"
#include "Tween/TweenValue.h"
#include "Tween/EaseType.h"
#include "Tween/TweenerHandle.h"
#include "Event/EventContext.h"
#include "FairyBlueprintLibrary.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FTweenUpdateDynDelegate, const FTweenValue&, Value, const FTweenValue&, DeltaValue);

UCLASS()
class FAIRYGUI_API UFairyBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "FairyGUI")
    static const FUIConfig& GetUIConfig();

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    static void SetUIConfig(const FUIConfig& InConfig);

    UFUNCTION(BlueprintPure, Category = "FairyGUI|Variant")
    static bool GetVariantAsBool(UPARAM(ref) FNVariant& InVariant);

    UFUNCTION(BlueprintPure, Category = "FairyGUI|Variant")
    static int32 GetVariantAsInt(UPARAM(ref) FNVariant& InVariant);

    UFUNCTION(BlueprintPure, Category = "FairyGUI|Variant")
    static float GetVariantAsFloat(UPARAM(ref) FNVariant& InVariant);

    UFUNCTION(BlueprintPure, Category = "FairyGUI|Variant")
    static FString GetVariantAsString(UPARAM(ref) FNVariant& InVariant);

    UFUNCTION(BlueprintPure, Category = "FairyGUI|Variant")
    static FColor GetVariantAsColor(UPARAM(ref) FNVariant& InVariant);

    UFUNCTION(BlueprintPure, Category = "FairyGUI|Variant", meta = (DeterminesOutputType = "ClassType"))
    static UObject* GetVariantAsUObject(UPARAM(ref) FNVariant& InVariant, TSubclassOf<UObject> ClassType);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI|Variant")
    static FNVariant& SetVariantBool(UPARAM(ref) FNVariant& InVariant, bool bInValue);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI|Variant")
    static FNVariant& SetVariantInt(UPARAM(ref) FNVariant& InVariant, int32 InValue);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI|Variant")
    static FNVariant& SetVariantFloat(UPARAM(ref) FNVariant& InVariant, float InValue);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI|Variant")
    static FNVariant& SetVariantString(UPARAM(ref) FNVariant& InVariant, const FString& InValue);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI|Variant")
    static FNVariant& SetVariantColor(UPARAM(ref) FNVariant& InVariant, const FColor& InValue);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI|Variant")
    static FNVariant& SetVariantUObject(UPARAM(ref) FNVariant& InVariant, UObject* InValue);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI|Tween", meta = (AutoCreateRefTerm = "OnUpdate,OnComplete"))
    static FTweenerHandle TweenFloat(float StartValue, float EndValue, EEaseType EaseType, float Duration, int32 Repeat, const FTweenUpdateDynDelegate& OnUpdate, const FSimpleDynDelegate& OnComplete);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI|Tween", meta = (AutoCreateRefTerm = "OnUpdate,OnComplete"))
    static FTweenerHandle TweenVector2(const FVector2D& StartValue, const FVector2D& EndValue, EEaseType EaseType, float Duration, int32 Repeat, const FTweenUpdateDynDelegate& OnUpdate, const FSimpleDynDelegate& OnComplete);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI|Tween")
    static void KillTween(UPARAM(ref) FTweenerHandle& Handle, bool bSetComplete = false);

    UFUNCTION(BlueprintCallable, Category = "FairyGUI")
    static void SetPackageItemExtension(const FString& URL, TSubclassOf<UGComponent> ClassType);
};