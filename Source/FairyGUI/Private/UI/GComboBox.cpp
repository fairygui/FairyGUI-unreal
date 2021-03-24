#include "UI/GComboBox.h"
#include "UI/UIPackage.h"
#include "UI/GTextField.h"
#include "UI/GTextInput.h"
#include "UI/GLabel.h"
#include "UI/GButton.h"
#include "UI/GController.h"
#include "UI/GList.h"
#include "UI/GRoot.h"
#include "Utils/ByteBuffer.h"

UGComboBox::UGComboBox() :
    bItemsUpdated(true),
    SelectedIndex(-1)
{
    VisibleItemCount = FUIConfig::Config.DefaultComboBoxVisibleItemCount;
}

UGComboBox::~UGComboBox()
{
}

const FString& UGComboBox::GetText() const
{
    if (TitleObject != nullptr)
        return TitleObject->GetText();
    else
        return G_EMPTY_STRING;
}

void UGComboBox::SetText(const FString& InText)
{
    if (TitleObject != nullptr)
        TitleObject->SetText(InText);
    UpdateGear(6);
}

const FString& UGComboBox::GetIcon() const
{
    if (IconObject != nullptr)
        return IconObject->GetIcon();
    else
        return G_EMPTY_STRING;
}

void UGComboBox::SetIcon(const FString & InIcon)
{
    if (IconObject != nullptr)
        IconObject->SetIcon(InIcon);
    UpdateGear(7);
}

FColor UGComboBox::GetTitleColor() const
{
    UGTextField* TextField = GetTextField();
    if (TextField)
        return TextField->GetTextFormat().Color;
    else
        return FColor::Black;
}

void UGComboBox::SetTitleColor(const FColor & InColor)
{
    UGTextField* TextField = GetTextField();
    if (TextField)
    {
        TextField->GetTextFormat().Color = InColor;
        TextField->ApplyFormat();
    }
}

int32 UGComboBox::GetTitleFontSize() const
{
    UGTextField* TextField = GetTextField();
    if (TextField)
        return TextField->GetTextFormat().Size;
    else
        return 0;
}

void UGComboBox::SetTitleFontSize(int32 InFontSize)
{
    UGTextField* TextField = GetTextField();
    if (TextField)
    {
        TextField->GetTextFormat().Size = InFontSize;
        TextField->ApplyFormat();
    }
}

const FString& UGComboBox::GetValue() const
{
    if (SelectedIndex >= 0 && SelectedIndex < Values.Num())
        return Values[SelectedIndex];
    else
        return G_EMPTY_STRING;
}

void UGComboBox::SetValue(const FString& InValue)
{
    SetSelectedIndex(Values.Find(InValue));
}

void UGComboBox::SetSelectedIndex(int32 InIndex)
{
    if (SelectedIndex == InIndex)
        return;

    SelectedIndex = InIndex;
    if (SelectedIndex >= 0 && SelectedIndex < Items.Num())
    {
        SetText(Items[SelectedIndex]);
        if (Icons.Num() > 0 && SelectedIndex != -1 && SelectedIndex < Icons.Num())
            SetIcon(Icons[SelectedIndex]);
    }
    else
    {
        SetTitle(G_EMPTY_STRING);
        if (Icons.Num() > 0)
            SetIcon(G_EMPTY_STRING);
    }

    UpdateSelectionController();
}

void UGComboBox::Refresh()
{
    if (Items.Num() > 0)
    {
        if (SelectedIndex >= Items.Num())
            SelectedIndex = Items.Num() - 1;
        else if (SelectedIndex == -1)
            SelectedIndex = 0;
        SetTitle(Items[SelectedIndex]);
    }
    else
    {
        SetTitle(G_EMPTY_STRING);
        SelectedIndex = -1;
    }

    if (Icons.Num() > 0)
    {
        if (SelectedIndex != -1 && SelectedIndex < Icons.Num())
            SetIcon(Icons[SelectedIndex]);
        else
            SetIcon(G_EMPTY_STRING);
    }

    bItemsUpdated = true;
}

void UGComboBox::SetState(const FString& InState)
{
    if (ButtonController != nullptr)
        ButtonController->SetSelectedPage(InState);
}

void UGComboBox::SetCurrentState()
{
    if (IsGrayed() && ButtonController != nullptr && ButtonController->HasPage(UGButton::DISABLED))
        SetState(UGButton::DISABLED);
    else if (DropdownObject != nullptr && DropdownObject->GetParent() != nullptr)
        SetState(UGButton::DOWN);
    else
        SetState(bOver ? UGButton::OVER : UGButton::UP);
}

void UGComboBox::UpdateSelectionController()
{
    if (SelectionController != nullptr && !SelectionController->bChanging && SelectedIndex < SelectionController->GetPageCount())
    {
        UGController* c = SelectionController;
        SelectionController = nullptr;
        c->SetSelectedIndex(SelectedIndex);
        SelectionController = c;
    }
}

void UGComboBox::UpdateDropdownList()
{
    if (bItemsUpdated)
    {
        bItemsUpdated = false;
        RenderDropdownList();
        ListObject->ResizeToFit(VisibleItemCount);
    }
}

void UGComboBox::ShowDropdown()
{
    UpdateDropdownList();
    if (ListObject->GetSelectionMode() == EListSelectionMode::Single)
        ListObject->SetSelectedIndex(-1);
    DropdownObject->SetWidth(Size.X);
    ListObject->EnsureBoundsCorrect();

    GetUIRoot()->TogglePopup(DropdownObject, this, PopupDirection);
    if (DropdownObject->GetParent() != nullptr)
        SetState(UGButton::DOWN);
}

void UGComboBox::RenderDropdownList()
{
    ListObject->RemoveChildrenToPool();
    int32 cnt = Items.Num();
    for (int32 i = 0; i < cnt; i++)
    {
        UGObject* Obj = ListObject->AddItemFromPool();
        Obj->SetText(Items[i]);
        Obj->SetIcon((Icons.Num() > 0 && i < Icons.Num()) ? Icons[i] : G_EMPTY_STRING);
        Obj->Name = i < Values.Num() ? Values[i] : G_EMPTY_STRING;
    }
}

void UGComboBox::HandleControllerChanged(UGController* Controller)
{
    UGComponent::HandleControllerChanged(Controller);

    if (SelectionController == Controller)
        SetSelectedIndex(Controller->GetSelectedIndex());
}

void UGComboBox::HandleGrayedChanged()
{
    if (ButtonController != nullptr && ButtonController->HasPage(UGButton::DISABLED))
    {
        if (IsGrayed())
            SetState(UGButton::DISABLED);
        else
            SetState(UGButton::UP);
    }
    else
        UGComponent::HandleGrayedChanged();
}

UGTextField* UGComboBox::GetTextField() const
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

FNVariant UGComboBox::GetProp(EObjectPropID PropID) const
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

void UGComboBox::SetProp(EObjectPropID PropID, const FNVariant& InValue)
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

void UGComboBox::ConstructExtension(FByteBuffer* Buffer)
{
    Buffer->Seek(0, 6);

    ButtonController = GetController("button");
    TitleObject = GetChild("title");
    IconObject = GetChild("icon");

    const FString& dropdownResource = Buffer->ReadS();
    if (!dropdownResource.IsEmpty())
    {
        DropdownObject = Cast<UGComponent>(UUIPackage::CreateObjectFromURL(dropdownResource, this));
        verifyf(DropdownObject != nullptr, TEXT("should be a component."));

        ListObject = Cast<UGList>(DropdownObject->GetChild("list"));
        verifyf(ListObject != nullptr, TEXT("should container a list component named list."));

        ListObject->On(FUIEvents::ClickItem).AddUObject(this, &UGComboBox::OnClickItem);

        ListObject->AddRelation(DropdownObject, ERelationType::Width);
        ListObject->RemoveRelation(DropdownObject, ERelationType::Height);

        DropdownObject->AddRelation(ListObject, ERelationType::Height);
        DropdownObject->RemoveRelation(ListObject, ERelationType::Width);

        DropdownObject->On(FUIEvents::RemovedFromStage).AddUObject(this, &UGComboBox::OnPopupWinClosed);
    }

    On(FUIEvents::RollOver).AddUObject(this, &UGComboBox::OnRollOverHandler);
    On(FUIEvents::RollOut).AddUObject(this, &UGComboBox::OnRollOutHandler);
    On(FUIEvents::TouchBegin).AddUObject(this, &UGComboBox::OnTouchBeginHandler);
    On(FUIEvents::TouchEnd).AddUObject(this, &UGComboBox::OnTouchEndHandler);
}

void UGComboBox::SetupAfterAdd(FByteBuffer* Buffer, int32 BeginPos)
{
    UGComponent::SetupAfterAdd(Buffer, BeginPos);

    if (!Buffer->Seek(BeginPos, 6))
        return;

    if ((EObjectType)Buffer->ReadByte() != PackageItem->ObjectType)
        return;

    const FString* str;
    bool hasIcon = false;
    int32 itemCount = Buffer->ReadShort();
    for (int32 i = 0; i < itemCount; i++)
    {
        int32 nextPos = Buffer->ReadShort();
        nextPos += Buffer->GetPos();

        Items.Add(Buffer->ReadS());
        Values.Add(Buffer->ReadS());
        if ((str = Buffer->ReadSP()) != nullptr)
        {
            if (!hasIcon)
            {
                for (int32 j = 0; j < Items.Num() - 1; j++)
                    Icons.Add(G_EMPTY_STRING);
            }
            Icons.Add(*str);
        }

        Buffer->SetPos(nextPos);
    }

    if ((str = Buffer->ReadSP()) != nullptr)
    {
        SetTitle(*str);
        SelectedIndex = Items.Find(*str);
    }
    else if (Items.Num() > 0)
    {
        SelectedIndex = 0;
        SetTitle(Items[0]);
    }
    else
        SelectedIndex = -1;

    if ((str = Buffer->ReadSP()) != nullptr)
        SetIcon(*str);

    if (Buffer->ReadBool())
        SetTitleColor(Buffer->ReadColor());
    int32 iv = Buffer->ReadInt();
    if (iv > 0)
        VisibleItemCount = iv;
    PopupDirection = (EPopupDirection)Buffer->ReadByte();

    iv = Buffer->ReadShort();
    if (iv >= 0)
        SelectionController = GetParent()->GetControllerAt(iv);
}

void UGComboBox::OnClickItem(UEventContext* Context)
{
    if (DropdownObject->GetParent()->IsA<UGRoot>())
        ((UGRoot*)DropdownObject->GetParent())->HidePopup(DropdownObject);
    SelectedIndex = INT_MIN;
    SetSelectedIndex(ListObject->GetChildIndex(Cast<UGObject>(Context->GetData().AsUObject())));

    DispatchEvent(FUIEvents::Changed);
}

void UGComboBox::OnRollOverHandler(UEventContext* Context)
{
    bOver = true;
    if (bDown || (DropdownObject != nullptr && DropdownObject->GetParent() != nullptr))
        return;

    SetCurrentState();
}

void UGComboBox::OnRollOutHandler(UEventContext* Context)
{
    bOver = false;
    if (bDown || (DropdownObject != nullptr && DropdownObject->GetParent() != nullptr))
        return;

    SetCurrentState();
}

void UGComboBox::OnTouchBeginHandler(UEventContext* Context)
{
    if (Context->GetMouseButton() != EKeys::LeftMouseButton)
        return;

    if (Context->GetInitiator()->IsA<UGTextInput>())
        return;

    bDown = true;

    if (DropdownObject != nullptr)
        ShowDropdown();

    Context->CaptureTouch();
}

void UGComboBox::OnTouchEndHandler(UEventContext* Context)
{
    if (Context->GetMouseButton() != EKeys::LeftMouseButton)
        return;

    if (bDown)
    {
        bDown = false;
        if (DropdownObject != nullptr && DropdownObject->GetParent() != nullptr)
            SetCurrentState();
    }
}

void UGComboBox::OnPopupWinClosed(UEventContext* Context)
{
    SetCurrentState();
}
