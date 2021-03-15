#include "UI/GSlider.h"
#include "Utils/ByteBuffer.h"

UGSlider::UGSlider()
{
}

UGSlider::~UGSlider()
{
}

void UGSlider::SetTitleType(EProgressTitleType InTitleType)
{
    if (TitleType != InTitleType)
    {
        TitleType = InTitleType;
        Update();
    }
}

void UGSlider::SetMin(float InMin)
{
    if (Min != InMin)
    {
        Min = InMin;
        Update();
    }
}

void UGSlider::SetMax(float InMax)
{
    if (Max != InMax)
    {
        Max = InMax;
        Update();
    }
}

void UGSlider::SetValue(float InValue)
{
    if (Value != InValue)
    {
        Value = InValue;
        Update();
    }
}

void UGSlider::SetWholeNumbers(bool InFlag)
{
    if (bWholeNumbers != InFlag)
    {
        bWholeNumbers = InFlag;
        Update();
    }
}

void UGSlider::Update()
{
    float Percent = FMath::Min((float)(Value / Max), 1.f);
    UpdateWithPercent(Percent, false);
}

void UGSlider::UpdateWithPercent(float Percent, bool bManual)
{
    Percent = FMath::Clamp(Percent, 0.f, 1.f);
    if (bManual)
    {
        float newValue = Min + (Max - Min) * Percent;
        if (newValue < Min)
            newValue = Min;
        if (newValue > Max)
            newValue = Max;
        if (bWholeNumbers)
        {
            newValue = round(newValue);
            Percent = FMath::Clamp((float)((newValue - Min) / (Max - Min)), 0.f, 1.f);
        }

        if (newValue != Value)
        {
            Value = newValue;
            if (DispatchEvent(FUIEvents::Changed))
                return;
        }
    }

    if (TitleObject != nullptr)
    {
        FString NewTitle;
        switch (TitleType)
        {
        case EProgressTitleType::Percent:
            NewTitle.AppendInt(FMath::FloorToInt(Percent * 100));
            NewTitle.AppendChar('%');
            break;

        case EProgressTitleType::ValueMax:
            NewTitle.AppendInt(FMath::FloorToInt(Value));
            NewTitle.AppendChar('/');
            NewTitle.AppendInt(FMath::FloorToInt(Max));
            break;

        case EProgressTitleType::Value:
            NewTitle.AppendInt(FMath::FloorToInt(Value));
            break;

        case EProgressTitleType::Max:
            NewTitle.AppendInt(FMath::FloorToInt(Max));
            break;
        default:
            break;
        }
        TitleObject->SetText(NewTitle);
    }

    FVector2D FullSize = GetSize() - BarMaxSizeDelta;;
    if (!bReverse)
    {
        if (BarObjectH != nullptr)
            BarObjectH->SetWidth(FMath::RoundToFloat(FullSize.X * Percent));

        if (BarObjectV != nullptr)
            BarObjectV->SetHeight(FMath::RoundToFloat(FullSize.Y * Percent));

    }
    else
    {
        if (BarObjectH != nullptr)
        {
            BarObjectH->SetWidth(FMath::RoundToFloat(FullSize.X * Percent));
            BarObjectH->SetX(BarStartPosition.X + (FullSize.X - BarObjectH->GetWidth()));
        }
        if (BarObjectV != nullptr)
        {
            BarObjectV->SetHeight(round(FullSize.Y * Percent));
            BarObjectV->SetY(BarStartPosition.Y + (FullSize.Y - BarObjectV->GetHeight()));
        }
    }
}

void UGSlider::HandleSizeChanged()
{
    UGComponent::HandleSizeChanged();

    BarMaxSize = GetSize() - BarMaxSizeDelta;

    if (!bUnderConstruct)
        Update();
}

void UGSlider::ConstructExtension(FByteBuffer* Buffer)
{
    TitleType = (EProgressTitleType)Buffer->ReadByte();
    bReverse = Buffer->ReadBool();
    if (Buffer->Version >= 2)
    {
        bWholeNumbers = Buffer->ReadBool();
        bChangeOnClick = Buffer->ReadBool();
    }

    TitleObject = GetChild("title");
    BarObjectH = GetChild("bar");
    BarObjectV = GetChild("bar_v");
    GripObject = GetChild("grip");

    if (BarObjectH != nullptr)
    {
        BarMaxSize.X = BarObjectH->GetWidth();
        BarMaxSizeDelta.X = GetWidth() - BarMaxSize.X;
        BarStartPosition.X = BarObjectH->GetX();
    }
    if (BarObjectV != nullptr)
    {
        BarMaxSize.Y = BarObjectV->GetHeight();
        BarMaxSizeDelta.Y = GetHeight() - BarMaxSize.Y;
        BarStartPosition.Y = BarObjectV->GetY();
    }

    if (GripObject != nullptr)
    {
        GripObject->On(FUIEvents::TouchBegin).AddUObject(this, &UGSlider::OnGripTouchBegin);
        GripObject->On(FUIEvents::TouchMove).AddUObject(this, &UGSlider::OnGripTouchMove);
    }

    On(FUIEvents::TouchBegin).AddUObject(this, &UGSlider::OnTouchBeginHandler);
}

void UGSlider::SetupAfterAdd(FByteBuffer* Buffer, int32 BeginPos)
{
    UGComponent::SetupAfterAdd(Buffer, BeginPos);

    if (!Buffer->Seek(BeginPos, 6))
    {
        Update();
        return;
    }

    if ((EObjectType)Buffer->ReadByte() != PackageItem->ObjectType)
    {
        Update();
        return;
    }

    Value = Buffer->ReadInt();
    Max = Buffer->ReadInt();
    if (Buffer->Version >= 2)
        Min = Buffer->ReadInt();

    Update();
}

void UGSlider::OnTouchBeginHandler(UEventContext* Context)
{
    if (!bChangeOnClick)
        return;

    if (Context->GetMouseButton() != EKeys::LeftMouseButton)
        return;

    FVector2D pt = GripObject->GlobalToLocal(Context->GetPointerPosition());
    float percent = FMath::Clamp((float)((Value - Min) / (Max - Min)), 0.f, 1.f);
    float delta = 0;
    if (BarObjectH != nullptr)
        delta = pt.X / BarMaxSize.X;
    if (BarObjectV != nullptr)
        delta = pt.Y / BarMaxSize.Y;
    if (bReverse)
        percent -= delta;
    else
        percent += delta;
    UpdateWithPercent(percent, true);
}

void UGSlider::OnGripTouchBegin(UEventContext* Context)
{
    if (Context->GetMouseButton() != EKeys::LeftMouseButton)
        return;

    bCanDrag = true;
    Context->StopPropagation();
    Context->CaptureTouch();

    ClickPos = GlobalToLocal(Context->GetPointerPosition());
    ClickPercent = FMath::Clamp((float)((Value - Min) / (Max - Min)), 0.f, 1.f);
}

void UGSlider::OnGripTouchMove(UEventContext* Context)
{
    if (!bCanDrag)
        return;

    FVector2D pt = GlobalToLocal(Context->GetPointerPosition());
    FVector2D delta = pt - ClickPos;
    if (bReverse)
    {
        delta.X = -delta.X;
        delta.Y = -delta.Y;
    }

    float percent;
    if (BarObjectH != nullptr)
        percent = ClickPercent + delta.X / BarMaxSize.X;
    else
        percent = ClickPercent + delta.Y / BarMaxSize.Y;
    UpdateWithPercent(percent, true);
}
