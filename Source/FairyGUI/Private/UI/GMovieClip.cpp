#include "UI/GMovieClip.h"
#include "Widgets/NTexture.h"
#include "Widgets/SMovieClip.h"
#include "Utils/ByteBuffer.h"

UGMovieClip::UGMovieClip()
{
    if (!HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
    {
        DisplayObject = Content = SNew(SMovieClip).GObject(this);
        DisplayObject->SetInteractable(false);
    }
}

UGMovieClip::~UGMovieClip()
{

}

void UGMovieClip::SetPlaySettings(int32 InStart, int32 InEnd, int32 InTimes, int32 InEndAt, const FSimpleDelegate& InCompleteCallback)
{
    Content->SetPlaySettings(InStart, InEnd, InTimes, InEndAt, InCompleteCallback);
}

bool UGMovieClip::IsPlaying() const
{
    return Content->IsPlaying();
}

void UGMovieClip::SetPlaying(bool bInPlaying)
{
    Content->SetPlaying(bInPlaying);
}

int32 UGMovieClip::GetFrame() const
{
    return Content->GetFrame();
}

void UGMovieClip::SetFrame(int32 InFrame)
{
    Content->SetFrame(InFrame);
}

float UGMovieClip::GetTimeScale() const
{
    return Content->GetTimeScale();
}

void UGMovieClip::SetTimeScale(float InTimeScale)
{
    Content->SetTimeScale(InTimeScale);
}

void UGMovieClip::Advance(float Time)
{
    Content->Advance(Time);
}

EFlipType UGMovieClip::GetFlip() const
{
    return Content->Graphics.GetFlip();
}

void UGMovieClip::SetFlip(EFlipType InFlip)
{
    Content->Graphics.SetFlip(InFlip);
}

FColor UGMovieClip::GetColor() const
{
    return Content->Graphics.GetColor();
}

void UGMovieClip::SetColor(const FColor& InColor)
{
    Content->Graphics.SetColor(InColor);
}

FNVariant UGMovieClip::GetProp(EObjectPropID PropID) const
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

void UGMovieClip::SetProp(EObjectPropID PropID, const FNVariant& InValue)
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

void UGMovieClip::ConstructFromResource()
{
    TSharedPtr<FPackageItem> contentItem = PackageItem->GetBranch();
    SourceSize = contentItem->Size;
    InitSize = SourceSize;

    contentItem = contentItem->GetHighResolution();
    contentItem->Load();

    Content->SetClipData(contentItem->MovieClipData);

    SetSize(SourceSize);
}

void UGMovieClip::SetupBeforeAdd(FByteBuffer* Buffer, int32 BeginPos)
{
    UGObject::SetupBeforeAdd(Buffer, BeginPos);

    Buffer->Seek(BeginPos, 5);

    if (Buffer->ReadBool())
        SetColor(Buffer->ReadColor());
    SetFlip((EFlipType)Buffer->ReadByte());
    SetFrame(Buffer->ReadInt());
    SetPlaying(Buffer->ReadBool());
}
