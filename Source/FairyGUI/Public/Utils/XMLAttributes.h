#pragma once

#include "CoreMinimal.h"

struct FXMLAttributes : TMap<FString, FString>
{
    const FString& Get(const FString& AttrName, const FString& DefaultValue = "");
    int32 GetInt(const FString& AttrName, int32 DefaultValue = 0);
    float GetFloat(const FString& AttrName, float DefaultValue = 0);
    bool GetBool(const FString& AttrName, bool DefaultValue = false);
    FColor GetColor(const FString& AttrName, const FColor& DefaultValue);
};