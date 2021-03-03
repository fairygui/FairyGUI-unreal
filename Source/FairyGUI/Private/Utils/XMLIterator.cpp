#include "Utils/XMLIterator.h"

extern const FString FAIRYGUI_API G_EMPTY_STRING;

const TCHAR* CDATA_START = TEXT("<![CDATA[");
const TCHAR* CDATA_END = TEXT("]]>");
const TCHAR* COMMENT_START = TEXT("<!--");
const TCHAR* COMMENT_END = TEXT("-->");
const TCHAR* SYMBOL_LT = TEXT("<");
const TCHAR* SYMBOL_AMP = TEXT("&");

void FXMLIterator::Begin(const FString& InText, bool bInLowerCaseName)
{
    Source = &InText;
    SourceLen = Source->Len();
    bLowerCaseName = bInLowerCaseName;
    ParsePos = 0;
    LastTagEnd = 0;
    TagPos = 0;
    TagLength = 0;
    TagName.Reset();
}

bool FXMLIterator::NextTag()
{
    int32 pos;
    TCHAR c;
    TagType = EXMLTagType::Start;
    FString buffer;
    LastTagEnd = ParsePos;
    bAttrParsed = false;
    LastTagName = TagName;
    const FString& Text = *Source;

    while ((pos = Text.Find(SYMBOL_LT, ESearchCase::IgnoreCase, ESearchDir::FromStart, ParsePos)) != -1)
    {
        ParsePos = pos;
        pos++;

        if (pos == SourceLen)
            break;

        c = Text[pos];
        if (c == '!')
        {
            if (SourceLen > pos + 7 && Text.Mid(pos - 1, 9) == CDATA_START)
            {
                pos = Text.Find(CDATA_END, ESearchCase::IgnoreCase, ESearchDir::FromStart, pos);
                TagType = EXMLTagType::CDATA;
                TagName.Reset();
                TagPos = ParsePos;
                if (pos == -1)
                    TagLength = SourceLen - ParsePos;
                else
                    TagLength = pos + 3 - ParsePos;
                ParsePos += TagLength;
                return true;
            }
            else if (SourceLen > pos + 2 && Text.Mid(pos - 1, 4) == COMMENT_START)
            {
                pos = Text.Find(COMMENT_END, ESearchCase::IgnoreCase, ESearchDir::FromStart, pos);
                TagType = EXMLTagType::Comment;
                TagName.Reset();
                TagPos = ParsePos;
                if (pos == -1)
                    TagLength = SourceLen - ParsePos;
                else
                    TagLength = pos + 3 - ParsePos;
                ParsePos += TagLength;
                return true;
            }
            else
            {
                pos++;
                TagType = EXMLTagType::Instruction;
            }
        }
        else if (c == '/')
        {
            pos++;
            TagType = EXMLTagType::End;
        }
        else if (c == '?')
        {
            pos++;
            TagType = EXMLTagType::Instruction;
        }

        for (; pos < SourceLen; pos++)
        {
            c = Text[pos];
            if (FChar::IsWhitespace(c) || c == '>' || c == '/')
                break;
        }
        if (pos == SourceLen)
            break;

        buffer.Append(*Text + ParsePos + 1, pos - ParsePos - 1);
        if (buffer.Len() > 0 && buffer[0] == '/')
            buffer.RemoveAt(0, 1);

        bool singleQuoted = false, doubleQuoted = false;
        int32 possibleEnd = -1;
        for (; pos < SourceLen; pos++)
        {
            c = Text[pos];
            if (c == '"')
            {
                if (!singleQuoted)
                    doubleQuoted = !doubleQuoted;
            }
            else if (c == '\'')
            {
                if (!doubleQuoted)
                    singleQuoted = !singleQuoted;
            }

            if (c == '>')
            {
                if (!(singleQuoted || doubleQuoted))
                {
                    possibleEnd = -1;
                    break;
                }

                possibleEnd = pos;
            }
            else if (c == '<')
                break;
        }
        if (possibleEnd != -1)
            pos = possibleEnd;

        if (pos == SourceLen)
            break;

        if (Text[pos - 1] == '/')
            TagType = EXMLTagType::Void;

        TagName = buffer;
        if (bLowerCaseName)
            TagName = TagName.ToLower();
        TagPos = ParsePos;
        TagLength = pos + 1 - ParsePos;
        ParsePos += TagLength;

        return true;
    }

    TagPos = SourceLen;
    TagLength = 0;
    TagName.Reset();
    return false;
}

FString FXMLIterator::GetTagSource() const
{
    return (*Source).Mid(TagPos, TagLength);
}

FString FXMLIterator::GetRawText(bool bTrim) const
{
    if (LastTagEnd == TagPos)
        return G_EMPTY_STRING;

    if (bTrim)
    {
        int32 i = LastTagEnd;
        for (; i < TagPos; i++)
        {
            char c = (*Source)[i];
            if (!FChar::IsWhitespace(c))
                break;
        }

        if (i == TagPos)
            return G_EMPTY_STRING;
        else
            return (*Source).Mid(i, TagPos - i).TrimEnd();
    }
    else
        return (*Source).Mid(LastTagEnd, TagPos - LastTagEnd);
}

FString FXMLIterator::GetText(bool bTrim) const
{
    if (LastTagEnd == TagPos)
        return G_EMPTY_STRING;

    if (bTrim)
    {
        int32 i = LastTagEnd;
        for (; i < TagPos; i++)
        {
            char c = (*Source)[i];
            if (!FChar::IsWhitespace(c))
                break;
        }

        if (i == TagPos)
            return G_EMPTY_STRING;
        else
            return DecodeString((*Source).Mid(i, TagPos - i).TrimEnd());
    }
    else
        return DecodeString((*Source).Mid(LastTagEnd, TagPos - LastTagEnd));
}

void FXMLIterator::ParseAttributes()
{
    if (bAttrParsed)
        return;

    bAttrParsed = true;

    FString attrName;
    int32 valueStart;
    int32 valueEnd;
    bool waitValue = false;
    int32 quoted;
    FString buffer;
    int32 i = TagPos;
    int32 attrEnd = TagPos + TagLength;

    if (i < attrEnd && (*Source)[i] == '<')
    {
        for (; i < attrEnd; i++)
        {
            TCHAR c = (*Source)[i];
            if (FChar::IsWhitespace(c) || c == '>' || c == '/')
                break;
        }
    }

    for (; i < attrEnd; i++)
    {
        char c = (*Source)[i];
        if (c == '=')
        {
            valueStart = -1;
            valueEnd = -1;
            quoted = 0;
            for (int32 j = i + 1; j < attrEnd; j++)
            {
                char c2 = (*Source)[j];
                if (FChar::IsWhitespace(c2))
                {
                    if (valueStart != -1 && quoted == 0)
                    {
                        valueEnd = j - 1;
                        break;
                    }
                }
                else if (c2 == '>')
                {
                    if (quoted == 0)
                    {
                        valueEnd = j - 1;
                        break;
                    }
                }
                else if (c2 == '"')
                {
                    if (valueStart != -1)
                    {
                        if (quoted != 1)
                        {
                            valueEnd = j - 1;
                            break;
                        }
                    }
                    else
                    {
                        quoted = 2;
                        valueStart = j + 1;
                    }
                }
                else if (c2 == '\'')
                {
                    if (valueStart != -1)
                    {
                        if (quoted != 2)
                        {
                            valueEnd = j - 1;
                            break;
                        }
                    }
                    else
                    {
                        quoted = 1;
                        valueStart = j + 1;
                    }
                }
                else if (valueStart == -1)
                {
                    valueStart = j;
                }
            }

            if (valueStart != -1 && valueEnd != -1)
            {
                attrName = buffer;
                if (bLowerCaseName)
                    attrName = attrName.ToLower();
                buffer.Reset();
                Attributes.Add(attrName, DecodeString((*Source).Mid(valueStart, valueEnd - valueStart + 1)));
                i = valueEnd + 1;
            }
            else
                break;
        }
        else if (!FChar::IsWhitespace(c))
        {
            if (waitValue || c == '/' || c == '>')
            {
                if (buffer.Len() > 0)
                {
                    attrName = buffer;
                    if (bLowerCaseName)
                        attrName = attrName.ToLower();
                    Attributes.Add(attrName, G_EMPTY_STRING);
                    buffer.Reset();
                }

                waitValue = false;
            }

            if (c != '/' && c != '>')
                buffer.AppendChar(c);
        }
        else
        {
            if (buffer.Len() > 0)
                waitValue = true;
        }
    }
}

FString FXMLIterator::DecodeString(const FString& InSource)
{
    int32 len = InSource.Len();
    int32 pos1 = 0, pos2 = 0;
    FString result;

    while (true)
    {
        pos2 = InSource.Find(SYMBOL_AMP, ESearchCase::IgnoreCase, ESearchDir::FromStart, pos1);
        if (pos2 == -1)
        {
            result.Append(InSource.Mid(pos1));
            break;
        }
        result.Append(InSource.Mid(pos1, pos2 - pos1));

        pos1 = pos2 + 1;
        pos2 = pos1;
        int32 end = FMath::Min(len, pos2 + 10);
        for (; pos2 < end; pos2++)
        {
            if (InSource[pos2] == ';')
                break;
        }
        if (pos2 < end && pos2 > pos1)
        {
            FString entity = InSource.Mid(pos1, pos2 - pos1);
            if (entity[0] == '#')
            {
                if (entity.Len() > 1)
                {
                    uint32 u;
                    if (entity[1] == 'x')
                        u = FParse::HexNumber(*entity.Mid(2));
                    else
                        u = FParse::HexNumber(*entity.Mid(1));
                    result.AppendChar((TCHAR)u);
                    pos1 = pos2 + 1;
                }
                else
                    result.AppendChar('&');
            }
            else
            {
                static const TMap<FString, TCHAR> EscapeCharacters = {
                    { "amp", '&' },
                    { "quot", '"'},
                    { "lt", '<' },
                    { "gt", '>'},
                };

                TCHAR c = EscapeCharacters.FindRef(entity);
                if (c != 0)
                {
                    result.AppendChar(c);
                    pos1 = pos2 + 1;
                }
                else
                    result.AppendChar('&');
            }
        }
        else
        {
            result.AppendChar('&');
        }
    }

    return result;
}
