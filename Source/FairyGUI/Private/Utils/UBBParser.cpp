#include "Utils/UBBParser.h"

extern const FString FAIRYGUI_API G_EMPTY_STRING;

FUBBParser FUBBParser::DefaultParser;

const FString LEFT_BRACKET = "[";
const FString RIGHT_BRACKET = "]";

FUBBParser::FUBBParser() :
    DefaultImgWidth(0),
    DefaultImgHeight(0),
    Source(nullptr),
    ReadPos(0)
{
    Handlers.Add("url", FTagHandler::CreateRaw(this, &FUBBParser::OnTag_URL));
    Handlers.Add("img", FTagHandler::CreateRaw(this, &FUBBParser::OnTag_IMG));
    Handlers.Add("b", FTagHandler::CreateRaw(this, &FUBBParser::OnTag_Simple));
    Handlers.Add("i", FTagHandler::CreateRaw(this, &FUBBParser::OnTag_Simple));
    Handlers.Add("u", FTagHandler::CreateRaw(this, &FUBBParser::OnTag_Simple));
    Handlers.Add("sup", FTagHandler::CreateRaw(this, &FUBBParser::OnTag_Simple));
    Handlers.Add("sub", FTagHandler::CreateRaw(this, &FUBBParser::OnTag_Simple));
    Handlers.Add("color", FTagHandler::CreateRaw(this, &FUBBParser::OnTag_COLOR));
    Handlers.Add("font", FTagHandler::CreateRaw(this, &FUBBParser::OnTag_FONT));
    Handlers.Add("size", FTagHandler::CreateRaw(this, &FUBBParser::OnTag_SIZE));
    Handlers.Add("align", FTagHandler::CreateRaw(this, &FUBBParser::OnTag_ALIGN));
}

FUBBParser::~FUBBParser()
{

}

FString FUBBParser::Parse(const FString& Text, bool bRemove)
{
    Source = &Text;
    ReadPos = 0;
    LastColor.Reset();
    LastFontSize.Reset();

    int32 pos1 = 0, pos2, pos3;
    bool bEnd;
    FString tag, attr;
    FString buffer;

    while ((pos2 = Text.Find(LEFT_BRACKET, ESearchCase::CaseSensitive, ESearchDir::FromStart, pos1)) != -1)
    {
        if (pos2 > 0 && Text[pos2 - 1] == '\\')
        {
            buffer.Append(*Text + pos1, pos2 - pos1 - 1);
            buffer.Append(LEFT_BRACKET);
            pos1 = pos2 + 1;
            continue;
        }

        buffer.Append(*Text + pos1, pos2 - pos1);
        pos1 = pos2;
        pos2 = Text.Find(RIGHT_BRACKET, ESearchCase::CaseSensitive, ESearchDir::FromStart, pos1);
        if (pos2 == -1)
            break;

        if (pos2 == pos1 + 1)
        {
            buffer.Append(*Text + pos1, 2);
            pos1 = pos2 + 1;
            continue;
        }

        bEnd = Text[pos1 + 1] == '/';
        pos3 = bEnd ? pos1 + 2 : pos1 + 1;
        tag = Text.Mid(pos3, pos2 - pos3);
        ReadPos = pos2 + 1;
        attr.Reset();
        if (tag.FindChar('=', pos3))
        {
            attr = tag.Mid(pos3 + 1);
            tag = tag.Mid(0, pos3);
        }
        tag = tag.ToLower();
        FTagHandler* func = Handlers.Find(tag);
        if (func != nullptr)
        {
            FString repl = (*func).Execute(tag, bEnd, attr);
            if(!bRemove)
                buffer.Append(repl);
        }
        else if (DefaultTagHandler.IsBound())
        {
            FString value;
            if (DefaultTagHandler.Execute(tag, bEnd, attr, value))
            {
                if (!bRemove)
                    buffer.Append(value);
            }
            else
                buffer.Append(*Text + pos1, pos2 - pos1 + 1);
        }
        else
        {
            buffer.Append(*Text + pos1, pos2 - pos1 + 1);
        }
        pos1 = ReadPos;
    }

    if (buffer.Len() == 0)
        return Text;
    else
    {
        if (pos1 < Text.Len())
            buffer.Append(*Text + pos1, Text.Len() - pos1);

        return buffer;
    }
}

FString FUBBParser::GetTagText(bool bRemove)
{
    int32 pos1 = ReadPos;
    int32 pos2;
    FString buffer;
    while ((pos2 = Source->Find(LEFT_BRACKET, ESearchCase::CaseSensitive, ESearchDir::FromStart, pos1)) != -1)
    {
        if ((*Source)[pos2 - 1] == '\\')
        {
            buffer.Append(**Source + pos1, pos2 - pos1 - 1);
            buffer.Append(LEFT_BRACKET);
            pos1 = pos2 + 1;
        }
        else
        {
            buffer.Append(**Source + pos1, pos2 - pos1);
            break;
        }
    }
    if (pos2 == -1)
        return G_EMPTY_STRING;

    if (bRemove)
        ReadPos = pos2;

    return buffer;
}

FString FUBBParser::OnTag_URL(const FString& TagName, bool bEnd, const FString& Attr)
{
    if (!bEnd)
    {
        if (!Attr.IsEmpty())
            return "<a href=\"" + Attr + "\" target=\"_blank\">";
        else
        {
            FString Href = GetTagText(false);
            return "<a href=\"" + Href + "\" target=\"_blank\">";
        }
    }
    else
        return"</a>";
}

FString FUBBParser::OnTag_IMG(const FString& TagName, bool bEnd, const FString& Attr)
{
    if (!bEnd)
    {
        FString src = GetTagText(true);
        if (src.IsEmpty())
            return src;

        if (DefaultImgWidth != 0)
            return "<img src=\"" + src + "\" width=\"" + FString::FromInt(DefaultImgWidth) + "\" height=\"" + FString::FromInt(DefaultImgHeight) + "\"/>";
        else
            return "<img src=\"" + src + "\"/>";
    }
    else
        return G_EMPTY_STRING;
}

FString FUBBParser::OnTag_Simple(const FString& TagName, bool bEnd, const FString& Attr)
{
    return bEnd ? ("</" + TagName + ">") : ("<" + TagName + ">");
}

FString FUBBParser::OnTag_COLOR(const FString& TagName, bool bEnd, const FString& Attr)
{
    if (!bEnd) {
        LastColor = Attr;
        return "<font color=\"" + Attr + "\">";
    }
    else
        return "</font>";
}

FString FUBBParser::OnTag_FONT(const FString& TagName, bool bEnd, const FString& Attr)
{
    if (!bEnd)
        return "<font face=\"" + Attr + "\">";
    else
        return "</font>";
}

FString FUBBParser::OnTag_SIZE(const FString& TagName, bool bEnd, const FString& Attr)
{
    if (!bEnd) {
        LastFontSize = Attr;
        return "<font size=\"" + Attr + "\">";
    }
    else
        return "</font>";
}

FString FUBBParser::OnTag_ALIGN(const FString& TagName, bool bEnd, const FString& Attr)
{
    if (!bEnd)
        return  "<p align=\"" + Attr + "\">";
    else
        return "</p>";
}
