#include "Utils/HTMLParser.h"
#include "Utils/XMLIterator.h"

enum class SupportedTagNames {
    INVALID, B, I, U, STRIKE, SUB, SUP, FONT, BR, IMG, A, INPUT, SELECT, P, UI, DIV, LI, HTML, BODY, HEAD, STYLE, SCRIPT, FORM
};

FHTMLParser FHTMLParser::DefaultParser;
FHTMLParseOptions FHTMLParser::DefaultParseOptions;

FHTMLParser::FHTMLParser()
{
}

void FHTMLParser::Parse(const FString& InText, const FNTextFormat& InFormat, TArray<FHTMLElement>& OutElements, const FHTMLParseOptions& InParseOptions)
{
    ParseOptions = InParseOptions;
    Elements = &OutElements;
    TextFormatStack.Reset();
    (FNTextFormat&)Format = InFormat;
    Format.bColorChanged = false;
    int32 skipText = 0;
    bool ignoreWhiteSpace = ParseOptions.bIgnoreWhiteSpace;
    bool skipNextCR = false;
    FString text;

    static const TMap<FString, SupportedTagNames> TagConstMap = {
        { "b", SupportedTagNames::B },
        { "i", SupportedTagNames::I },
        { "u", SupportedTagNames::U },
        { "strike", SupportedTagNames::STRIKE },
        { "sub", SupportedTagNames::SUP },
        { "font", SupportedTagNames::FONT },
        { "br", SupportedTagNames::BR },
        { "img", SupportedTagNames::IMG },
        { "a", SupportedTagNames::A },
        { "input", SupportedTagNames::INPUT },
        { "select", SupportedTagNames::SELECT },
        { "p", SupportedTagNames::P },
        { "ui", SupportedTagNames::UI },
        { "div", SupportedTagNames::DIV },
        { "li", SupportedTagNames::LI },
        { "html", SupportedTagNames::HTML },
        { "body", SupportedTagNames::BODY },
        { "head", SupportedTagNames::HEAD },
        { "style", SupportedTagNames::STYLE },
        { "script", SupportedTagNames::SCRIPT },
        { "form", SupportedTagNames::FORM },
    };

    FXMLIterator XMLIterator;
    XMLIterator.Begin(InText, true);
    while (XMLIterator.NextTag())
    {
        if (skipText == 0)
        {
            text = XMLIterator.GetText(ignoreWhiteSpace);
            if (text.Len() > 0)
            {
                if (skipNextCR && text[0] == '\n')
                    text = text.Mid(1);
                AppendText(text);
            }
        }

        skipNextCR = false;
        switch (TagConstMap.FindRef(XMLIterator.TagName))
        {
        case SupportedTagNames::B:
            if (XMLIterator.TagType == EXMLTagType::Start)
            {
                PushTextFormat();
                Format.bBold = true;
            }
            else
                PopTextFormat();
            break;

        case SupportedTagNames::I:
            if (XMLIterator.TagType == EXMLTagType::Start)
            {
                PushTextFormat();
                Format.bItalic = true;
            }
            else
                PopTextFormat();
            break;

        case SupportedTagNames::U:
            if (XMLIterator.TagType == EXMLTagType::Start)
            {
                PushTextFormat();
                Format.bUnderline = true;
            }
            else
                PopTextFormat();
            break;

        case SupportedTagNames::STRIKE:
            if (XMLIterator.TagType == EXMLTagType::Start)
            {
                PushTextFormat();
                //Format.strikethrough = true;
            }
            else
                PopTextFormat();
            break;

        case SupportedTagNames::SUB:
            break;

        case SupportedTagNames::SUP:
            break;

        case SupportedTagNames::FONT:
            if (XMLIterator.TagType == EXMLTagType::Start)
            {
                PushTextFormat();

                XMLIterator.ParseAttributes();
                Format.Size = XMLIterator.Attributes.GetInt("size", Format.Size);
                const FString& color = XMLIterator.Attributes.Get("color");
                if (color.Len() > 0)
                {
                    Format.Color = FColor::FromHex(color);
                    Format.bColorChanged = true;
                }
            }
            else if (XMLIterator.TagType == EXMLTagType::End)
                PopTextFormat();
            break;

        case SupportedTagNames::BR:
            AppendText("\n");
            break;

        case SupportedTagNames::IMG:
            if (XMLIterator.TagType == EXMLTagType::Start || XMLIterator.TagType == EXMLTagType::Void)
            {
                XMLIterator.ParseAttributes();

                FHTMLElement element;
                element.Type = EHTMLElementType::Image;
                element.Attributes.Append(XMLIterator.Attributes);
                element.Name = element.Attributes.Get("name");
                element.Format.Align = Format.Align;
                Elements->Add(MoveTemp(element));
            }
            break;

        case SupportedTagNames::A:
            if (XMLIterator.TagType == EXMLTagType::Start)
            {
                PushTextFormat();

                Format.bUnderline = Format.bUnderline || ParseOptions.bLinkUnderline;
                if (!Format.bColorChanged && ParseOptions.LinkColor.A != 0)
                    Format.Color = ParseOptions.LinkColor;

                FHTMLElement element;
                element.Type = EHTMLElementType::Link;
                XMLIterator.ParseAttributes();
                element.Attributes.Append(XMLIterator.Attributes);
                element.Name = element.Attributes.Get("name");
                element.Format.Align = Format.Align;
                Elements->Add(MoveTemp(element));
            }
            else if (XMLIterator.TagType == EXMLTagType::End)
            {
                PopTextFormat();

                FHTMLElement element;
                element.Type = EHTMLElementType::LinkEnd;
                Elements->Add(MoveTemp(element));
            }
            break;

        case SupportedTagNames::INPUT:
        {
            FHTMLElement element;
            element.Type = EHTMLElementType::Input;
            XMLIterator.ParseAttributes();
            element.Attributes.Append(XMLIterator.Attributes);
            element.Name = element.Attributes.Get("name");
            element.Format = Format;
            Elements->Add(element);
        }
        break;

        case SupportedTagNames::SELECT:
        {
            if (XMLIterator.TagType == EXMLTagType::Start || XMLIterator.TagType == EXMLTagType::Void)
            {
                FHTMLElement element;
                element.Type = EHTMLElementType::Select;
                XMLIterator.ParseAttributes();
                if (XMLIterator.TagType == EXMLTagType::Start)
                {
                    FString Items, Values;
                    while (XMLIterator.NextTag())
                    {
                        if (XMLIterator.TagName == "select")
                            break;

                        if (XMLIterator.TagName == "option")
                        {
                            if (XMLIterator.TagType == EXMLTagType::Start || XMLIterator.TagType == EXMLTagType::Void)
                            {
                                if (!Values.IsEmpty())
                                    Values.AppendChar(',');
                                Values.Append(XMLIterator.Attributes.Get("value"));
                            }
                            else
                            {
                                if (!Items.IsEmpty())
                                    Items.AppendChar(',');
                                Items.Append(XMLIterator.GetText());
                            }
                        }
                    }
                    element.Attributes.Add("items", Items);
                    element.Attributes.Add("values", Values);
                }
                element.Name = element.Attributes.Get("name");
                element.Format = Format;
                Elements->Add(element);
            }
        }
        break;

        case SupportedTagNames::P:
            if (XMLIterator.TagType == EXMLTagType::Start)
            {
                PushTextFormat();
                const FString& align = XMLIterator.Attributes.Get("align");
                if (align == "center")
                    Format.Align = EAlignType::Center;
                else if (align == "right")
                    Format.Align = EAlignType::Right;

                if (!IsNewLine())
                    AppendText("\n");
            }
            else if (XMLIterator.TagType == EXMLTagType::End)
            {
                AppendText("\n");
                skipNextCR = true;

                PopTextFormat();
            }
            break;

        case SupportedTagNames::UI:
        case SupportedTagNames::DIV:
        case SupportedTagNames::LI:
            if (XMLIterator.TagType == EXMLTagType::Start)
            {
                if (!IsNewLine())
                    AppendText("\n");
            }
            else
            {
                AppendText("\n");
                skipNextCR = true;
            }
            break;

        case SupportedTagNames::HTML:
        case SupportedTagNames::BODY:
            //full html
            ignoreWhiteSpace = true;
            break;

        case SupportedTagNames::HEAD:
        case SupportedTagNames::STYLE:
        case SupportedTagNames::SCRIPT:
        case SupportedTagNames::FORM:
            if (XMLIterator.TagType == EXMLTagType::Start)
                skipText++;
            else if (XMLIterator.TagType == EXMLTagType::End)
                skipText--;
            break;
        }
    }

    if (skipText == 0)
    {
        text = XMLIterator.GetText(ignoreWhiteSpace);
        if (text.Len() > 0)
        {
            if (skipNextCR && text[0] == '\n')
                text = text.Mid(1);
            AppendText(text);
        }
    }
}

void FHTMLParser::PushTextFormat()
{
    TextFormatStack.Add(Format);
}

void FHTMLParser::PopTextFormat()
{
    Format = TextFormatStack.Pop();
}

bool FHTMLParser::IsNewLine()
{
    if (Elements->Num() > 0)
    {
        const FHTMLElement& element = Elements->Last();
        if (element.Type == EHTMLElementType::Text)
            return element.Text.EndsWith("\n");
        else
            return false;
    }

    return true;
}

void FHTMLParser::AppendText(const FString& Text)
{
    if (Elements->Num() > 0)
    {
        FHTMLElement& element = Elements->Last();
        if (element.Type == EHTMLElementType::Text && element.Format.EqualStyle(Format))
        {
            element.Text.Append(Text);
            return;
        }
    }

    {
        FHTMLElement element;
        element.Type = EHTMLElementType::Text;
        element.Text = Text;
        element.Format = Format;
        Elements->Add(element);
    }
}