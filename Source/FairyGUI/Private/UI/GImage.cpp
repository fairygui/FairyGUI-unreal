#include "UI/GImage.h"
#include "Utils/ByteBuffer.h"
#include "Widgets/NTexture.h"
#include "Widgets/SFImage.h"

UGImage::UGImage()
{
    if (!HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
    {
        DisplayObject = Content = SNew(SFImage).GObject(this);
        DisplayObject->SetInteractable(false);
    }
}

UGImage::~UGImage()
{

}

EFlipType UGImage::GetFlip() const
{
    return Content->Graphics.GetFlip();
}

void UGImage::SetFlip(EFlipType InFlip)
{
    Content->Graphics.SetFlip(InFlip);
}

FColor UGImage::GetColor() const
{
    return Content->Graphics.GetColor();
}

void UGImage::SetColor(const FColor& InColor)
{
    Content->Graphics.SetColor(InColor);
}

EFillMethod UGImage::GetFillMethod() const
{
    return Content->GetFillMethod();
}

void UGImage::SetFillMethod(EFillMethod Method)
{
    Content->SetFillMethod(Method);
}

int32 UGImage::GetFillOrigin() const
{
    return Content->GetFillOrigin();
}

void UGImage::SetFillOrigin(int32 Origin)
{
    Content->SetFillOrigin(Origin);
}

bool UGImage::IsFillClockwise() const
{
    return Content->IsFillClockwise();
}

void UGImage::SetFillClockwise(bool bClockwise)
{
    Content->SetFillClockwise(bClockwise);
}

float UGImage::GetFillAmount() const
{
    return Content->GetFillAmount();
}

void UGImage::SetFillAmount(float Amount)
{
    Content->SetFillAmount(Amount);
}

FNVariant UGImage::GetProp(EObjectPropID PropID) const
{
    switch (PropID)
    {
    case EObjectPropID::Color:
        return FNVariant(GetColor());
    default:
        return UGObject::GetProp(PropID);
    }
}

void UGImage::SetProp(EObjectPropID PropID, const FNVariant& InValue)
{
    switch (PropID)
    {
    case EObjectPropID::Color:
        SetColor(InValue.AsColor());
        break;
    default:
        UGObject::SetProp(PropID, InValue);
        break;
    }
}

void UGImage::ConstructFromResource()
{
    TSharedPtr<FPackageItem> ContentItem = PackageItem->GetBranch();
    InitSize = SourceSize = ContentItem->Size;

    ContentItem = ContentItem->GetHighResolution();
    ContentItem->Load();

    Content->SetTexture(ContentItem->Texture);
    if (ContentItem->Scale9Grid.IsSet())
        Content->SetScale9Grid(ContentItem->Scale9Grid);
    else if (ContentItem->bScaleByTile)
        Content->SetScaleByTile(true);

    SetSize(SourceSize);
}

void UGImage::SetupBeforeAdd(FByteBuffer* Buffer, int32 BeginPos)
{
    UGObject::SetupBeforeAdd(Buffer, BeginPos);

    Buffer->Seek(BeginPos, 5);

    if (Buffer->ReadBool())
        SetColor(Buffer->ReadColor());
    SetFlip((EFlipType)Buffer->ReadByte());
    int32 method = Buffer->ReadByte();
    if (method != 0)
    {
        Content->SetFillMethod((EFillMethod)method);
        Content->SetFillOrigin(Buffer->ReadByte());
        Content->SetFillClockwise(Buffer->ReadBool());
        Content->SetFillAmount(Buffer->ReadFloat());
    }
}
