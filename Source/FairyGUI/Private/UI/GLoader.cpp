#include "UI/GLoader.h"
#include "UI/UIPackage.h"
#include "UI/GComponent.h"
#include "Widgets/NTexture.h"
#include "Widgets/SMovieClip.h"
#include "Widgets/SContainer.h"
#include "Utils/ByteBuffer.h"
#include "Engine/AssetManager.h"

UGLoader::UGLoader()
{
    if (!HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
    {
        DisplayObject = Container = SNew(SContainer).GObject(this);
        Content = SNew(SMovieClip);
        Content->SetInteractable(false);
        Container->AddChild(Content.ToSharedRef());
    }
}

UGLoader::~UGLoader()
{

}

void UGLoader::SetURL(const FString& InURL)
{
    if (URL.Compare(InURL, ESearchCase::CaseSensitive) == 0)
        return;

    ClearContent();
    URL = InURL;
    LoadContent();
    UpdateGear(7);
}

void UGLoader::SetAlign(EAlignType InAlign)
{
    if (Align != InAlign)
    {
        Align = InAlign;
        UpdateLayout();
    }
}

void UGLoader::SetVerticalAlign(EVerticalAlignType InVerticalAlign)
{
    if (VerticalAlign != InVerticalAlign)
    {
        VerticalAlign = InVerticalAlign;
        UpdateLayout();
    }
}

void UGLoader::SetAutoSize(bool bInAutoSize)
{
    if (bAutoSize != bInAutoSize)
    {
        bAutoSize = bInAutoSize;
        UpdateLayout();
    }
}

void UGLoader::SetFill(ELoaderFillType InFillType)
{
    if (Fill != InFillType)
    {
        Fill = InFillType;
        UpdateLayout();
    }
}

void UGLoader::SetShrinkOnly(bool bInShrinkOnly)
{
    if (bShrinkOnly != bInShrinkOnly)
    {
        bShrinkOnly = bInShrinkOnly;
        UpdateLayout();
    }
}

EFlipType UGLoader::GetFlip() const
{
    return Content->Graphics.GetFlip();
}

void UGLoader::SetFlip(EFlipType InFlip)
{
    Content->Graphics.SetFlip(InFlip);
}

FColor UGLoader::GetColor() const
{
    return Content->Graphics.GetColor();
}

void UGLoader::SetColor(const FColor& InColor)
{
    Content->Graphics.SetColor(InColor);
}

EFillMethod UGLoader::GetFillMethod() const
{
    return Content->GetFillMethod();
}

void UGLoader::SetFillMethod(EFillMethod Method)
{
    Content->SetFillMethod(Method);
}

int32 UGLoader::GetFillOrigin() const
{
    return Content->GetFillOrigin();
}

void UGLoader::SetFillOrigin(int32 Origin)
{
    Content->SetFillOrigin(Origin);
}

bool UGLoader::IsFillClockwise() const
{
    return Content->IsFillClockwise();
}

void UGLoader::SetFillClockwise(bool bClockwise)
{
    Content->SetFillClockwise(bClockwise);
}

float UGLoader::GetFillAmount() const
{
    return Content->GetFillAmount();
}

void UGLoader::SetFillAmount(float Amount)
{
    Content->SetFillAmount(Amount);
}

bool UGLoader::IsPlaying() const
{
    return Content->IsPlaying();
}

void UGLoader::SetPlaying(bool bInPlaying)
{
    if (Content->IsPlaying() != bInPlaying)
    {
        Content->SetPlaying(bInPlaying);
        UpdateGear(5);
    }
}

int UGLoader::GetFrame() const
{
    return Content->GetFrame();
}

void UGLoader::SetFrame(int32 InFrame)
{
    if (Content->GetFrame() != InFrame)
    {
        Content->SetFrame(InFrame);
        UpdateGear(5);
    }
}

void UGLoader::LoadContent()
{
    ClearContent();

    if (URL.IsEmpty())
        return;

    if (URL.StartsWith("ui://"))
        LoadFromPackage(URL);
    else
        LoadExternal();
}

void UGLoader::ClearContent()
{
    ContentItem.Reset();
    Content->SetTexture(nullptr);
    Content->SetClipData(nullptr);
    if (Content2 != nullptr)
    {
        Container->RemoveChild(Content2->GetDisplayObject());
        Content2 = nullptr;
    }
}

void UGLoader::LoadFromPackage(const FString& ItemURL)
{
    ContentItem = UUIPackage::GetItemByURL(ItemURL);
    if (ContentItem.IsValid())
    {
        ContentItem = ContentItem->GetBranch();
        SourceSize = ContentItem->Size;
        ContentItem = ContentItem->GetHighResolution();
        ContentItem->Load();

        if (ContentItem->Type == EPackageItemType::Image)
        {
            Content->SetTexture(ContentItem->Texture);
            if (ContentItem->Scale9Grid.IsSet())
                Content->SetScale9Grid(ContentItem->Scale9Grid);
            else if (ContentItem->bScaleByTile)
                Content->SetScaleByTile(true);

            UpdateLayout();
        }
        else if (ContentItem->Type == EPackageItemType::MovieClip)
        {
            Content->SetClipData(ContentItem->MovieClipData);
            UpdateLayout();
        }
        else if (ContentItem->Type == EPackageItemType::Component)
        {
            UGObject* obj = UUIPackage::CreateObjectFromURL(ItemURL, this);
            if (obj == nullptr || !obj->IsA<UGComponent>())
                SetErrorState();
            else
            {
                Content2 = Cast<UGComponent>(obj);
                Container->AddChild(Content2->GetDisplayObject());
                UpdateLayout();
            }
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

void UGLoader::LoadExternal()
{
    SoftObjectPath = MakeShareable(new FSoftObjectPath(URL));

    UAssetManager::GetStreamableManager().RequestAsyncLoad(*SoftObjectPath, FStreamableDelegate::CreateUObject(this, &UGLoader::OnExternalLoaded, URL));
}

void UGLoader::OnExternalLoaded(FString LoadingURL)
{
    if (!SoftObjectPath.IsValid() || LoadingURL != URL)
        return;

    TSoftObjectPtr<UTexture2D> NativeTexture(*SoftObjectPath);
    UNTexture* NTexture = NewObject<UNTexture>(this);
    NTexture->Init(NativeTexture.Get());
    Content->SetTexture(NTexture);
    Content->SetNativeSize();
    SourceSize = NTexture->GetSize();
    UpdateLayout();

    SoftObjectPath.Reset();
}

void UGLoader::UpdateLayout()
{
    if (Content2 == nullptr && Content->GetTexture() == nullptr && !Content->GetClipData().IsValid())
    {
        if (bAutoSize)
        {
            bUpdatingLayout = true;
            SetSize(FVector2D(50, 30));
            bUpdatingLayout = false;
        }
        return;
    }

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
            if (Content2 != nullptr)
            {
                Content2->SetPosition(FVector2D(0, 0));
                Content2->SetScale(FVector2D(1, 1));
            }
            else
            {
                Content->SetPosition(FVector2D(0, 0));
                Content->SetSize(contentSize);
            }

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

    if (Content2 != nullptr)
        Content2->SetScale(ContentScale);
    else
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

    if (Content2 != nullptr)
        Content2->SetPosition(ContentPosition);
    else
        Content->SetPosition(ContentPosition);
}

void UGLoader::SetErrorState()
{

}

FNVariant UGLoader::GetProp(EObjectPropID PropID) const
{
    switch (PropID)
    {
    case EObjectPropID::Color:
        return FNVariant(GetColor());
    case EObjectPropID::Playing:
        return FNVariant(Content->IsPlaying());
    case EObjectPropID::Frame:
        return FNVariant(Content->GetFrame());
    case EObjectPropID::TimeScale:
        return FNVariant(Content->GetTimeScale());
    default:
        return UGObject::GetProp(PropID);
    }
}

void UGLoader::SetProp(EObjectPropID PropID, const FNVariant& InValue)
{
    switch (PropID)
    {
    case EObjectPropID::Color:
        SetColor(InValue.AsColor());
        break;
    case EObjectPropID::Playing:
        Content->SetPlaying(InValue.AsBool());
        break;
    case EObjectPropID::Frame:
        Content->SetFrame(InValue.AsInt());
        break;
    case EObjectPropID::TimeScale:
        Content->SetTimeScale(InValue.AsFloat());
        break;
    case EObjectPropID::DeltaTime:
        Content->Advance(InValue.AsFloat());
        break;
    default:
        UGObject::SetProp(PropID, InValue);
        break;
    }
}

void UGLoader::HandleSizeChanged()
{
    UGObject::HandleSizeChanged();

    if (!bUpdatingLayout)
        UpdateLayout();
}

void UGLoader::SetupBeforeAdd(FByteBuffer* Buffer, int32 BeginPos)
{
    UGObject::SetupBeforeAdd(Buffer, BeginPos);

    Buffer->Seek(BeginPos, 5);

    URL = Buffer->ReadS();
    Align = (EAlignType)Buffer->ReadByte();
    VerticalAlign = (EVerticalAlignType)Buffer->ReadByte();
    Fill = (ELoaderFillType)Buffer->ReadByte();
    bShrinkOnly = Buffer->ReadBool();
    bAutoSize = Buffer->ReadBool();
    bShowErrorSign = Buffer->ReadBool();
    Content->SetPlaying(Buffer->ReadBool());
    Content->SetFrame(Buffer->ReadInt());

    if (Buffer->ReadBool())
        SetColor(Buffer->ReadColor());
    int32 method = Buffer->ReadByte();
    if (method != 0)
    {
        Content->SetFillMethod((EFillMethod)method);
        Content->SetFillOrigin(Buffer->ReadByte());
        Content->SetFillClockwise(Buffer->ReadBool());
        Content->SetFillAmount(Buffer->ReadFloat());
    }

    if (!URL.IsEmpty())
        LoadContent();
}