#include "UI/GButton.h"
#include "UI/GTextField.h"
#include "UI/GLabel.h"
#include "UI/GController.h"
#include "Utils/ByteBuffer.h"
#include "FairyApplication.h"

const FString UGButton::UP = "up";
const FString UGButton::DOWN = "down";
const FString UGButton::OVER = "over";
const FString UGButton::SELECTED_OVER = "selectedOver";
const FString UGButton::DISABLED = "disabled";
const FString UGButton::SELECTED_DISABLED = "selectedDisabled";

UGButton::UGButton() :
    bChangeStateOnClick(true),
    DownEffectValue(0.8f)
{
    Sound = FUIConfig::Config.ButtonSound;
    SoundVolumeScale = FUIConfig::Config.ButtonSoundVolumeScale;
}

UGButton::~UGButton()
{
}

void UGButton::SetText(const FString& InText)
{
    Title = InText;
    if (TitleObject != nullptr)
        TitleObject->SetText(InText);
    UpdateGear(6);
}

const FString& UGButton::GetIcon() const
{
    if (IconObject != nullptr)
        return IconObject->GetIcon();
    else
        return G_EMPTY_STRING;
}

void UGButton::SetIcon(const FString & InIcon)
{
    if (IconObject != nullptr)
        IconObject->SetIcon(InIcon);
    UpdateGear(7);
}

void UGButton::SetSelectedTitle(const FString& InTitle)
{
    SelectedTitle = InTitle;
    if (TitleObject != nullptr)
        TitleObject->SetText((bSelected && SelectedTitle.Len() > 0) ? SelectedTitle : Title);
}

void UGButton::SetSelectedIcon(const FString& InIcon)
{
    SelectedIcon = InIcon;
    if (IconObject != nullptr)
        IconObject->SetIcon((bSelected && SelectedIcon.Len() > 0) ? SelectedIcon : Icon);
}

FColor UGButton::GetTitleColor() const
{
    UGTextField* TextField = GetTextField();
    if (TextField)
        return TextField->GetTextFormat().Color;
    else
        return FColor::Black;
}

void UGButton::SetTitleColor(const FColor & InColor)
{
    UGTextField* TextField = GetTextField();
    if (TextField)
    {
        TextField->GetTextFormat().Color = InColor;
        TextField->ApplyFormat();
    }
}

int32 UGButton::GetTitleFontSize() const
{
    UGTextField* TextField = GetTextField();
    if (TextField)
        return TextField->GetTextFormat().Size;
    else
        return 0;
}

void UGButton::SetTitleFontSize(int32 InFontSize)
{
    UGTextField* TextField = GetTextField();
    if (TextField)
    {
        TextField->GetTextFormat().Size = InFontSize;
        TextField->ApplyFormat();
    }
}

void UGButton::SetSelected(bool bInSelected)
{
    if (Mode == EButtonMode::Common)
        return;

    if (bSelected != bInSelected)
    {
        bSelected = bInSelected;
        SetCurrentState();
        if (!SelectedTitle.IsEmpty() && TitleObject != nullptr)
            TitleObject->SetText(bSelected ? SelectedTitle : Title);
        if (!SelectedIcon.IsEmpty())
        {
            const FString& str = bSelected ? SelectedIcon : Icon;
            if (IconObject != nullptr)
                IconObject->SetIcon(str);
        }
        if (RelatedController != nullptr && GetParent() != nullptr && !GetParent()->bBuildingDisplayList)
        {
            if (bSelected)
            {
                RelatedController->SetSelectedPageID(RelatedPageID);
                if (RelatedController->bAutoRadioGroupDepth)
                    GetParent()->AdjustRadioGroupDepth(this, RelatedController);
            }
            else if (Mode == EButtonMode::Check && RelatedController->GetSelectedPageID() == RelatedPageID)
                RelatedController->SetOppositePageID(RelatedPageID);
        }
    }
}

void UGButton::SetRelatedController(UGController* InController)
{
    RelatedController = InController;
}

void UGButton::SetState(const FString& InState)
{
    if (ButtonController != nullptr)
        ButtonController->SetSelectedPage(InState);

    if (DownEffect == 1)
    {
        int32 cnt = this->NumChildren();
        if (InState == DOWN || InState == SELECTED_OVER || InState == SELECTED_DISABLED)
        {
            int32 c = DownEffectValue * 255;
            FNVariant Color(FColor(c, c, c, 255));
            for (int32 i = 0; i < cnt; i++)
            {
                UGObject* Obj = this->GetChildAt(i);
                if (!Obj->IsA<UGTextField>())
                    Obj->SetProp(EObjectPropID::Color, Color);
            }
        }
        else
        {
            FNVariant Color(FColor::White);
            for (int32 i = 0; i < cnt; i++)
            {
                UGObject* Obj = this->GetChildAt(i);
                if (!Obj->IsA<UGTextField>())
                    Obj->SetProp(EObjectPropID::Color, Color);
            }
        }
    }
    else if (DownEffect == 2)
    {
        if (InState == DOWN || InState == SELECTED_OVER || InState == SELECTED_DISABLED)
        {
            if (!bDownScaled)
            {
                bDownScaled = true;
                SetScale(GetScale() * DownEffectValue);
            }
        }
        else
        {
            if (bDownScaled)
            {
                bDownScaled = false;
                SetScale(GetScale() / DownEffectValue);
            }
        }
    }
}

void UGButton::SetCurrentState()
{
    if (IsGrayed() && ButtonController != nullptr && ButtonController->HasPage(DISABLED))
    {
        if (bSelected)
            SetState(SELECTED_DISABLED);
        else
            SetState(DISABLED);
    }
    else
    {
        if (bSelected)
            SetState(bOver ? SELECTED_OVER : DOWN);
        else
            SetState(bOver ? OVER : UP);
    }
}

UGTextField * UGButton::GetTextField() const
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

FNVariant UGButton::GetProp(EObjectPropID PropID) const
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
    case EObjectPropID::Selected:
        return FNVariant(IsSelected());
    default:
        return UGComponent::GetProp(PropID);
    }
}

void UGButton::SetProp(EObjectPropID PropID, const FNVariant& InValue)
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
    case EObjectPropID::Selected:
        SetSelected(InValue.AsBool());
        break;
    default:
        UGComponent::SetProp(PropID, InValue);
        break;
    }
}

void UGButton::ConstructExtension(FByteBuffer* Buffer)
{
    Buffer->Seek(0, 6);

    Mode = (EButtonMode)Buffer->ReadByte();
    Buffer->ReadS(Sound);
    SoundVolumeScale = Buffer->ReadFloat();
    DownEffect = Buffer->ReadByte();
    DownEffectValue = Buffer->ReadFloat();
    if (DownEffect == 2)
        SetPivot(FVector2D(0.5f, 0.5f), IsPivotAsAnchor());

    ButtonController = GetController("button");
    TitleObject = GetChild("title");
    IconObject = GetChild("icon");
    if (TitleObject != nullptr)
        Title = TitleObject->GetText();
    if (IconObject != nullptr)
        Icon = IconObject->GetIcon();

    if (Mode == EButtonMode::Common)
        SetState(UP);

    On(FUIEvents::RollOver).AddUObject(this, &UGButton::OnRollOverHandler);
    On(FUIEvents::RollOut).AddUObject(this, &UGButton::OnRollOutHandler);
    On(FUIEvents::TouchBegin).AddUObject(this, &UGButton::OnTouchBeginHandler);
    On(FUIEvents::TouchEnd).AddUObject(this, &UGButton::OnTouchEndHandler);
    On(FUIEvents::Click).AddUObject(this, &UGButton::OnClickHandler);
    On(FUIEvents::RemovedFromStage).AddUObject(this, &UGButton::OnRemovedFromStageHandler);
}

void UGButton::SetupAfterAdd(FByteBuffer* Buffer, int32 BeginPos)
{
    UGComponent::SetupAfterAdd(Buffer, BeginPos);

    if (!Buffer->Seek(BeginPos, 6))
        return;

    if ((EObjectType)Buffer->ReadByte() != PackageItem->ObjectType)
        return;

    const FString* str;

    if ((str = Buffer->ReadSP()) != nullptr)
        SetTitle(*str);
    if ((str = Buffer->ReadSP()) != nullptr)
        SetSelectedTitle(*str);
    if ((str = Buffer->ReadSP()) != nullptr)
        SetIcon(*str);
    if ((str = Buffer->ReadSP()) != nullptr)
        SetSelectedIcon(*str);
    if (Buffer->ReadBool())
        SetTitleColor(Buffer->ReadColor());
    int32 iv = Buffer->ReadInt();
    if (iv != 0)
        SetTitleFontSize(iv);
    iv = Buffer->ReadShort();
    if (iv >= 0)
        RelatedController = GetParent()->GetControllerAt(iv);
    RelatedPageID = Buffer->ReadS();

    Buffer->ReadS(Sound);
    if (Buffer->ReadBool())
        SoundVolumeScale = Buffer->ReadFloat();

    SetSelected(Buffer->ReadBool());
}

void UGButton::HandleControllerChanged(UGController* Controller)
{
    UGObject::HandleControllerChanged(Controller);

    if (RelatedController == Controller)
        SetSelected(RelatedPageID == Controller->GetSelectedPageID());
}

void UGButton::OnRollOverHandler(UEventContext* Context)
{
    if (ButtonController == nullptr || !ButtonController->HasPage(OVER))
        return;

    bOver = true;
    if (bDown)
        return;

    if (IsGrayed() && ButtonController->HasPage(DISABLED))
        return;

    SetState(bSelected ? SELECTED_OVER : OVER);
}

void UGButton::OnRollOutHandler(UEventContext* Context)
{
    if (ButtonController == nullptr || !ButtonController->HasPage(OVER))
        return;

    bOver = false;
    if (bDown)
        return;

    if (IsGrayed() && ButtonController->HasPage(DISABLED))
        return;

    SetState(bSelected ? DOWN : UP);
}

void UGButton::OnTouchBeginHandler(UEventContext* Context)
{
    if (Context->GetMouseButton() != EKeys::LeftMouseButton)
        return;

    bDown = true;
    Context->CaptureTouch();

    if (Mode == EButtonMode::Common)
    {
        if (IsGrayed() && ButtonController != nullptr && ButtonController->HasPage(DISABLED))
            SetState(SELECTED_DISABLED);
        else
            SetState(DOWN);
    }
}

void UGButton::OnTouchEndHandler(UEventContext* Context)
{
    if (Context->GetMouseButton() != EKeys::LeftMouseButton)
        return;

    if (bDown)
    {
        bDown = false;
        if (Mode == EButtonMode::Common)
        {
            if (IsGrayed() && ButtonController != nullptr && ButtonController->HasPage(DISABLED))
                SetState(DISABLED);
            else if (bOver)
                SetState(OVER);
            else
                SetState(UP);
        }
        else
        {
            if (!bOver && ButtonController != nullptr && (ButtonController->GetSelectedPage() == OVER || ButtonController->GetSelectedPage() == SELECTED_OVER))
            {
                SetCurrentState();
            }
        }
    }
}

void UGButton::OnClickHandler(UEventContext* Context)
{
    if (!Sound.IsEmpty())
        GetApp()->PlaySound(Sound, SoundVolumeScale);

    if (Mode == EButtonMode::Check)
    {
        if (bChangeStateOnClick)
        {
            SetSelected(!bSelected);
            DispatchEvent(FUIEvents::Changed);
        }
    }
    else if (Mode == EButtonMode::Radio)
    {
        if (bChangeStateOnClick && !bSelected)
        {
            SetSelected(true);
            DispatchEvent(FUIEvents::Changed);
        }
    }
    else
    {
        if (RelatedController != nullptr)
            RelatedController->SetSelectedPageID(RelatedPageID);
    }
}

void UGButton::OnRemovedFromStageHandler(UEventContext* Context)
{
    if (bOver)
        OnRollOutHandler(Context);
}
