#include "UI/GLoader3D.h"
#include "UI/UIPackage.h"
#include "UI/GComponent.h"
#include "Widgets/NTexture.h"
#include "Widgets/SMovieClip.h"
#include "Widgets/SContainer.h"
#include "Utils/ByteBuffer.h"

UGLoader3D::UGLoader3D()
{
    if (!HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
    {
        DisplayObject = Content = SNew(SFImage).GObject(this);
    }
}

UGLoader3D::~UGLoader3D()
{

}

void UGLoader3D::SetURL(const FString& InURL)
{
    if (URL.Compare(InURL, ESearchCase::CaseSensitive) == 0)
        return;

    ClearContent();
    URL = InURL;
    LoadContent();
    UpdateGear(7);
}

FColor UGLoader3D::GetColor() const
{
    return FColor();// Content->getColor();
}

void UGLoader3D::SetColor(const FColor& InColor)
{
    //Content->setColor(value);
}

void UGLoader3D::LoadContent()
{
    ClearContent();

    if (URL.IsEmpty())
        return;

    if (URL.StartsWith("ui://"))
        LoadFromPackage(URL);
    else
        LoadExternal();
}

void UGLoader3D::ClearContent()
{

}

void UGLoader3D::LoadFromPackage(const FString& ItemURL)
{
    ContentItem = UUIPackage::GetItemByURL(ItemURL);

    if (ContentItem.IsValid())
    {
        ContentItem = ContentItem->GetBranch();
        SourceSize = ContentItem->Size;
        ContentItem = ContentItem->GetHighResolution();
        ContentItem->Load();

        if (ContentItem->Type == EPackageItemType::Spine)
        {
            UpdateLayout();
        }
        else if (ContentItem->Type == EPackageItemType::DragonBones)
        {
            UpdateLayout();
        }
        else
        {
            if (bAutoSize)
                SetSize(ContentItem->Size);

            SetErrorState();

            UE_LOG(LogFairyGUI, Warning, TEXT("Unsupported type of GLoader: %d"), ContentItem->Type);
        }
    }
    else
        SetErrorState();
}

void UGLoader3D::LoadExternal()
{

}

void UGLoader3D::UpdateLayout()
{
    FVector2D contentSize = SourceSize;

    if (bAutoSize)
    {
        bUpdatingLayout = true;
        if (contentSize.X == 0)
            contentSize.X = 50;
        if (contentSize.Y == 0)
            contentSize.Y = 30;
        SetSize(contentSize);

        bUpdatingLayout = false;

        if (Size == contentSize)
        {
            Content->SetPosition(FVector2D(0, 0));
            Content->SetSize(contentSize);
            return;
        }
    }

    FVector2D ContentScale(1, 1);
    if (Fill != ELoaderFillType::None)
    {
        ContentScale = Size / SourceSize;

        if (ContentScale != FVector2D(1, 1))
        {
            if (Fill == ELoaderFillType::ScaleMatchHeight)
                ContentScale.X = ContentScale.Y;
            else if (Fill == ELoaderFillType::ScaleMatchWidth)
                ContentScale.Y = ContentScale.X;
            else if (Fill == ELoaderFillType::Scale)
            {
                if (ContentScale.X > ContentScale.Y)
                    ContentScale.X = ContentScale.Y;
                else
                    ContentScale.Y = ContentScale.X;
            }
            else if (Fill == ELoaderFillType::ScaleNoBorder)
            {
                if (ContentScale.X > ContentScale.Y)
                    ContentScale.Y = ContentScale.X;
                else
                    ContentScale.X = ContentScale.Y;
            }

            if (bShrinkOnly)
            {
                if (ContentScale.X > 1)
                    ContentScale.X = 1;
                if (ContentScale.Y > 1)
                    ContentScale.Y = 1;
            }

            contentSize = SourceSize * ContentScale;
        }
    }

    Content->SetSize(contentSize);

    FVector2D ContentPosition;
    if (Align == EAlignType::Center)
        ContentPosition.X = (Size.X - contentSize.X) / 2;
    else if (Align == EAlignType::Right)
        ContentPosition.X = Size.X - contentSize.X;
    else
        ContentPosition.X = 0;

    if (VerticalAlign == EVerticalAlignType::Middle)
        ContentPosition.Y = (Size.Y - contentSize.Y) / 2;
    else if (VerticalAlign == EVerticalAlignType::Bottom)
        ContentPosition.Y = Size.Y - contentSize.Y;
    else
        ContentPosition.Y = 0;

    Content->SetPosition(ContentPosition);
}

void UGLoader3D::SetErrorState()
{

}


FNVariant UGLoader3D::GetProp(EObjectPropID PropID) const
{
    switch (PropID)
    {
    case EObjectPropID::Color:
        return FNVariant(GetColor());
        break;
    default:
        return UGObject::GetProp(PropID);
    }
}

void UGLoader3D::SetProp(EObjectPropID PropID, const FNVariant& InValue)
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

void UGLoader3D::HandleSizeChanged()
{
    UGObject::HandleSizeChanged();

    if (!bUpdatingLayout)
        UpdateLayout();
}

void UGLoader3D::SetupBeforeAdd(FByteBuffer* Buffer, int32 BeginPos)
{
    UGObject::SetupBeforeAdd(Buffer, BeginPos);

    Buffer->Seek(BeginPos, 5);
}