#include "UI/GLabel.h"
#include "UI/GTextInput.h"
#include "UI/GButton.h"
#include "UI/GTextField.h"
#include "Utils/ByteBuffer.h"

UGLabel::UGLabel()
{
}

UGLabel::~UGLabel()
{
}

const FString& UGLabel::GetText() const
{
    if (TitleObject != nullptr)
        return TitleObject->GetText();
    else
        return G_EMPTY_STRING;
}

void UGLabel::SetText(const FString& InText)
{
    if (TitleObject != nullptr)
        TitleObject->SetText(InText);
    UpdateGear(6);
}

const FString& UGLabel::GetIcon() const
{
    if (IconObject != nullptr)
        return IconObject->GetIcon();
    else
        return G_EMPTY_STRING;
}

void UGLabel::SetIcon(const FString & InIcon)
{
    if (IconObject != nullptr)
        IconObject->SetIcon(InIcon);
    UpdateGear(7);
}

FColor UGLabel::GetTitleColor() const
{
    UGTextField* TextField = GetTextField();
    if (TextField)
        return TextField->GetTextFormat().Color;
    else
        return FColor::Black;
}

void UGLabel::SetTitleColor(const FColor& InColor)
{
    UGTextField* TextField = GetTextField();
    if (TextField)
    {
        TextField->GetTextFormat().Color = InColor;
        TextField->ApplyFormat();
    }
}

int32 UGLabel::GetTitleFontSize() const
{
    UGTextField* TextField = GetTextField();
    if (TextField)
        return TextField->GetTextFormat().Size;
    else
        return 0;
}

void UGLabel::SetTitleFontSize(int32 InFontSize)
{
    UGTextField* TextField = GetTextField();
    if (TextField)
    {
        TextField->GetTextFormat().Size = InFontSize;
        TextField->ApplyFormat();
    }
}

UGTextField * UGLabel::GetTextField() const
{
    if (TitleObject->IsA<UGTextField>())
        return Cast<UGTextField>(TitleObject);
    else if (TitleObject->IsA<UGLabel>())
        return Cast<UGLabel>(TitleObject)->GetTextField();
    else if (TitleObject->IsA<UGButton>())
        return Cast<UGButton>(TitleObject)->GetTextField();
    else
        return nullptr;
}

FNVariant UGLabel::GetProp(EObjectPropID PropID) const
{
    switch (PropID)
    {
    case EObjectPropID::Color:
        return FNVariant(GetTitleColor());
    case EObjectPropID::OutlineColor:
    {
        UGTextField* TextField = GetTextField();
        if (TextField != nullptr)
            return FNVariant(TextField->GetTextFormat().OutlineColor);
        else
            return FNVariant(FColor::Black);
    }
    case EObjectPropID::FontSize:
        return FNVariant(GetTitleFontSize());
    default:
        return UGComponent::GetProp(PropID);
    }
}

void UGLabel::SetProp(EObjectPropID PropID, const FNVariant& InValue)
{
    switch (PropID)
    {
    case EObjectPropID::Color:
        SetTitleColor(InValue.AsColor());
        break;
    case EObjectPropID::OutlineColor:
    {
        UGTextField* TextField = GetTextField();
        if (TextField != nullptr)
        {
            TextField->GetTextFormat().OutlineColor = InValue.AsColor();
            TextField->ApplyFormat();
        }
        break;
    }
    case EObjectPropID::FontSize:
        SetTitleFontSize(InValue.AsInt());
        break;
    default:
        UGComponent::SetProp(PropID, InValue);
        break;
    }
}

void UGLabel::ConstructExtension(FByteBuffer* Buffer)
{
    TitleObject = GetChild("title");
    IconObject = GetChild("icon");
}

void UGLabel::SetupAfterAdd(FByteBuffer* Buffer, int32 BeginPos)
{
    UGComponent::SetupAfterAdd(Buffer, BeginPos);

    if (!Buffer->Seek(BeginPos, 6))
        return;

    if ((EObjectType)Buffer->ReadByte() != PackageItem->ObjectType)
        return;

    const FString* str;

    if ((str = Buffer->ReadSP()) != nullptr)
        SetText(*str);
    if ((str = Buffer->ReadSP()) != nullptr)
        SetIcon(*str);
    if (Buffer->ReadBool())
        SetTitleColor(Buffer->ReadColor());
    int32 iv = Buffer->ReadInt();
    if (iv != 0)
        SetTitleFontSize(iv);

    if (Buffer->ReadBool())
    {
        UGTextInput* input = Cast<UGTextInput>(GetTextField());
        if (input)
        {
            if ((str = Buffer->ReadSP()) != nullptr)
                input->SetPrompt(*str);
            if ((str = Buffer->ReadSP()) != nullptr)
                input->SetRestrict(*str);
            iv = Buffer->ReadInt();
            if (iv != 0)
                input->SetMaxLength(iv);
            iv = Buffer->ReadInt();
            if (iv != 0)
                input->SetKeyboardType(iv);
            if (Buffer->ReadBool())
                input->SetPassword(true);
        }
        else
            Buffer->Skip(13);
    }
}

