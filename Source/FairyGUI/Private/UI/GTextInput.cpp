#include "UI/GTextInput.h"
#include "Utils/ByteBuffer.h"
#include "Utils/UBBParser.h"
#include "Widgets/STextInput.h"

UGTextInput::UGTextInput()
{
    if (!HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
    {
        DisplayObject = Content = SNew(STextInput).GObject(this);
        Content->SetOnTextChanged(FOnTextChanged::CreateLambda([this](const FText& InText) {
            Text = InText.ToString();
        }));
        Content->SetOnTextCommitted(FOnTextCommitted::CreateLambda([this](const FText& InText, ETextCommit::Type InType) {
            if (InType == ETextCommit::OnEnter)
                DispatchEvent(FUIEvents::Submit);
        }));
    }
}

UGTextInput::~UGTextInput()
{

}

TSharedRef<SMultiLineEditableText> UGTextInput::GetInputWidget() const
{
    return StaticCastSharedRef<SMultiLineEditableText>(Content->Widget);
}

void UGTextInput::SetText(const FString& InText)
{
    Content->Widget->SetText(FText::FromString(InText));
}

bool UGTextInput::IsSingleLine() const
{
    return false;
}

void UGTextInput::SetSingleLine(bool bFlag)
{
    Content->SetSingleLine(bFlag);
}

void UGTextInput::SetTextFormat(const FNTextFormat& InTextFormat)
{
    Content->SetTextFormat(InTextFormat);
}

void UGTextInput::ApplyFormat()
{
    Content->SetTextFormat(TextFormat);
}

void UGTextInput::SetPrompt(const FString& InPrompt)
{
    Content->Widget->SetHintText(FText::FromString(FUBBParser::DefaultParser.Parse(InPrompt, true)));
}

void UGTextInput::SetPassword(bool bInPassword)
{
    Content->SetPassword(bInPassword);
}

void UGTextInput::SetKeyboardType(int32 InKeyboardType)
{

}

void UGTextInput::SetMaxLength(int32 InMaxLength)
{
}

void UGTextInput::SetRestrict(const FString& InRestrict)
{

}

void UGTextInput::NotifyTextChanged(const FText& InText)
{
    Text = InText.ToString();
}

FNVariant UGTextInput::GetProp(EObjectPropID PropID) const
{
    switch (PropID)
    {
    case EObjectPropID::Color:
        return FNVariant(TextFormat.Color);
    case EObjectPropID::OutlineColor:
        return FNVariant(TextFormat.OutlineColor);
    case EObjectPropID::FontSize:
        return FNVariant(TextFormat.Size);
    default:
        return UGObject::GetProp(PropID);
    }
}

void UGTextInput::SetProp(EObjectPropID PropID, const FNVariant& InValue)
{
    switch (PropID)
    {
    case EObjectPropID::Color:
        TextFormat.Color = InValue.AsColor();
        ApplyFormat();
        break;
    case EObjectPropID::OutlineColor:
        TextFormat.OutlineColor = InValue.AsColor();
        ApplyFormat();
        break;
    case EObjectPropID::FontSize:
        TextFormat.Size = InValue.AsInt();
        ApplyFormat();
        break;
    default:
        UGObject::SetProp(PropID, InValue);
        break;
    }
}

void UGTextInput::SetupBeforeAdd(FByteBuffer* Buffer, int32 BeginPos)
{
    UGObject::SetupBeforeAdd(Buffer, BeginPos);

    Buffer->Seek(BeginPos, 5);

    TextFormat.Face = Buffer->ReadS();
    TextFormat.Size = Buffer->ReadShort();
    TextFormat.Color = Buffer->ReadColor();
    TextFormat.Align = (EAlignType)Buffer->ReadByte();
    TextFormat.VerticalAlign = (EVerticalAlignType)Buffer->ReadByte();
    TextFormat.LineSpacing = Buffer->ReadShort();
    TextFormat.LetterSpacing = Buffer->ReadShort();
    Buffer->ReadBool(); //bUBBEnabled
    Buffer->ReadByte(); //AutoSize
    TextFormat.bUnderline = Buffer->ReadBool();
    TextFormat.bItalic = Buffer->ReadBool();
    TextFormat.bBold = Buffer->ReadBool();
    if (Buffer->ReadBool())
        SetSingleLine(true);
    if (Buffer->ReadBool())
    {
        TextFormat.OutlineColor = Buffer->ReadColor();
        TextFormat.OutlineSize = Buffer->ReadFloat();
    }

    if (Buffer->ReadBool())
    {
        TextFormat.ShadowColor = Buffer->ReadColor();
        float f1 = Buffer->ReadFloat();
        float f2 = Buffer->ReadFloat();
        TextFormat.ShadowOffset = FVector2D(f1, -f2);
    }

    Buffer->ReadBool(); //TemplateVars;

    Buffer->Seek(BeginPos, 4);

    const FString* str;
    if ((str = Buffer->ReadSP()) != nullptr)
        SetPrompt(*str);

    if ((str = Buffer->ReadSP()) != nullptr)
        SetRestrict(*str);

    int32 iv = Buffer->ReadInt();
    if (iv != 0)
        SetMaxLength(iv);
    iv = Buffer->ReadInt();
    if (iv != 0)
        SetKeyboardType(iv);
    if (Buffer->ReadBool())
        SetPassword(true);
}

void UGTextInput::SetupAfterAdd(FByteBuffer* Buffer, int32 BeginPos)
{
    UGObject::SetupAfterAdd(Buffer, BeginPos);

    ApplyFormat();

    Buffer->Seek(BeginPos, 6);

    const FString& str = Buffer->ReadS();
    SetText(str);
}
