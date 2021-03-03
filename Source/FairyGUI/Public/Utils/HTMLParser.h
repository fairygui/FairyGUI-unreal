#pragma once

#include "HTMLElement.h"

struct FAIRYGUI_API FHTMLParseOptions
{
    bool bLinkUnderline;
    FColor LinkColor;
    FColor LinkBgColor;
    FColor LinkHoverBgColor;
    bool bIgnoreWhiteSpace;
};

class FAIRYGUI_API FHTMLParser
{
public:
    static FHTMLParser DefaultParser;
    static FHTMLParseOptions DefaultParseOptions;

    FHTMLParser();

    void Parse(const FString& InText, const FNTextFormat& InFormat, TArray<FHTMLElement>& OutElements, const FHTMLParseOptions& InParseOptions);

protected:
    void PushTextFormat();
    void PopTextFormat();
    bool IsNewLine();
    void AppendText(const FString& InText);

    struct FMyTextFormat : FNTextFormat
    {
        bool bColorChanged;
    };
    TArray<FMyTextFormat> TextFormatStack;
    FMyTextFormat Format;
    TArray<FHTMLElement>* Elements;
    FHTMLParseOptions ParseOptions;
};