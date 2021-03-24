#pragma once

#include "Slate.h"
#include "Widgets/NTextFormat.h"

DECLARE_DELEGATE_RetVal_ThreeParams(FString, FTagHandler, const FString&, bool, const FString&);
DECLARE_DELEGATE_RetVal_FourParams(bool, FDefaultTagHandler, const FString&, bool, const FString&, FString&);

class FAIRYGUI_API FUBBParser
{
public:
    static FUBBParser DefaultParser;

    FUBBParser();
    virtual ~FUBBParser();

    FString Parse(const FString& Text, bool bRemove = false);

    int32 DefaultImgWidth;
    int32 DefaultImgHeight;
    FString LastColor;
    FString LastFontSize;
    FDefaultTagHandler DefaultTagHandler;
    TMap<FString, FTagHandler> Handlers;

protected:
    virtual FString OnTag_URL(const FString& TagName, bool bEnd, const FString& Attr);
    virtual FString OnTag_IMG(const FString& TagName, bool bEnd, const FString& Attr);
    virtual FString OnTag_Simple(const FString& TagName, bool bEnd, const FString& Attr);
    virtual FString OnTag_COLOR(const FString& TagName, bool bEnd, const FString& Attr);
    virtual FString OnTag_FONT(const FString& TagName, bool bEnd, const FString& Attr);
    virtual FString OnTag_SIZE(const FString& TagName, bool bEnd, const FString& Attr);
    virtual FString OnTag_ALIGN(const FString& TagName, bool bEnd, const FString& Attr);

    FString GetTagText(bool bRemove);

    const FString* Source;
    int32 ReadPos;
};