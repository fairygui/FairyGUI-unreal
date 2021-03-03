#include "Widgets/SMovieClip.h"

FMovieClipData::FMovieClipData() :
    Interval(0),
    RepeatDelay(0),
    bSwing(false)
{
}

void FMovieClipData::AddReferencedObjects(FReferenceCollector& Collector)
{
    for (auto& Frame : Frames)
    {
        if (Frame.Texture != nullptr)
            Collector.AddReferencedObject(Frame.Texture);
    }
}

SMovieClip::SMovieClip() :
    Frame(0),
    bPlaying(true),
    TimeScale(1),
    Start(0),
    End(0),
    Times(0),
    EndAt(0),
    Status(0),
    FrameElapsed(0),
    bReversed(false),
    RepeatedCount(0)
{
}

void SMovieClip::Construct(const FArguments& InArgs)
{
    SDisplayObject::Construct(SDisplayObject::FArguments().GObject(InArgs._GObject));
}

void SMovieClip::SetClipData(TSharedPtr<FMovieClipData> InData)
{
    Data = InData;

    SetScale9Grid(TOptional<FBox2D>());
    SetScaleByTile(false);

    if (!Data.IsValid())
    {
        SetCanTick(false);
        Graphics.SetTexture(nullptr);
        return;
    }

    SetCanTick(bPlaying);
    int32 frameCount = Data->Frames.Num();

    if (End == -1 || End > frameCount - 1)
        End = frameCount - 1;
    if (EndAt == -1 || EndAt > frameCount - 1)
        EndAt = frameCount - 1;

    if (Frame < 0 || Frame > frameCount - 1)
        Frame = frameCount - 1;

    FrameElapsed = 0;
    RepeatedCount = 0;
    bReversed = false;

    DrawFrame();
}

void SMovieClip::SetPlaying(bool InPlaying)
{
    bPlaying = InPlaying;
    SetCanTick(bPlaying && Data.IsValid());
}

void SMovieClip::SetTimeScale(float InTimeScale)
{
    TimeScale = InTimeScale;
}

void SMovieClip::SetFrame(int32 InFrame)
{
    if (!Data.IsValid())
    {
        Frame = InFrame;
        return;
    }

    int32 frameCount = Data->Frames.Num();

    if (InFrame >= frameCount)
        InFrame = frameCount - 1;

    Frame = InFrame;
    FrameElapsed = 0;
    DrawFrame();
}

void SMovieClip::Advance(float Time)
{
    if (!Data.IsValid())
        return;

    int32 frameCount = Data->Frames.Num();
    if (frameCount == 0)
        return;

    int32 beginFrame = Frame;
    bool beginReversed = bReversed;
    float backupTime = Time;
    while (true)
    {
        float tt = Data->Interval + Data->Frames[Frame].AddDelay;
        if (Frame == 0 && RepeatedCount > 0)
            tt += Data->RepeatDelay;
        if (Time < tt)
        {
            FrameElapsed = 0;
            break;
        }

        Time -= tt;

        if (Data->bSwing)
        {
            if (bReversed)
            {
                Frame--;
                if (Frame <= 0)
                {
                    Frame = 0;
                    RepeatedCount++;
                    bReversed = !bReversed;
                }
            }
            else
            {
                Frame++;
                if (Frame > frameCount - 1)
                {
                    Frame = FMath::Max(0, frameCount - 2);
                    RepeatedCount++;
                    bReversed = !bReversed;
                }
            }
        }
        else
        {
            Frame++;
            if (Frame > frameCount - 1)
            {
                Frame = 0;
                RepeatedCount++;
            }
        }

        if (Frame == beginFrame && bReversed == beginReversed)
        {
            float roundTime = backupTime - Time;
            Time -= FMath::FloorToInt(Time / roundTime) * roundTime;
        }
    }
}

void SMovieClip::SetPlaySettings(int32 InStart, int32 InEnd, int32 InTimes, int32 InEndAt, const FSimpleDelegate& InCompleteCallback)
{
    int32 frameCount = Data.IsValid() ? Data->Frames.Num() : 0;

    Start = InStart;
    End = InEnd;
    if (End == -1 || End > frameCount - 1)
        End = frameCount - 1;
    Times = InTimes;
    EndAt = InEndAt;
    if (EndAt == -1)
        EndAt = End;
    Status = 0;
    CompleteCallback = InCompleteCallback;

    SetFrame(InStart);
}

void SMovieClip::DrawFrame()
{
    if (Data.IsValid() && Frame < Data->Frames.Num())
        Graphics.SetTexture(Data->Frames[Frame].Texture);
}

void SMovieClip::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
    SFImage::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

    if (!Data.IsValid() || !bPlaying)
        return;

    int32 frameCount = Data->Frames.Num();
    if (frameCount == 0 || Status == 3)
        return;

    float dt = InDeltaTime;
    if (TimeScale != 1)
        dt *= TimeScale;

    FrameElapsed += dt;
    float tt = Data->Interval + Data->Frames[Frame].AddDelay;
    if (Frame == 0 && RepeatedCount > 0)
        tt += Data->RepeatDelay;
    if (FrameElapsed < tt)
        return;

    FrameElapsed -= tt;
    if (FrameElapsed > Data->Interval)
        FrameElapsed = Data->Interval;

    if (Data->bSwing)
    {
        if (bReversed)
        {
            Frame--;
            if (Frame <= 0)
            {
                Frame = 0;
                RepeatedCount++;
                bReversed = !bReversed;
            }
        }
        else
        {
            Frame++;
            if (Frame > frameCount - 1)
            {
                Frame = FMath::Max(0, frameCount - 2);
                RepeatedCount++;
                bReversed = !bReversed;
            }
        }
    }
    else
    {
        Frame++;
        if (Frame > frameCount - 1)
        {
            Frame = 0;
            RepeatedCount++;
        }
    }

    if (Status == 1) //new loop
    {
        Frame = Start;
        FrameElapsed = 0;
        Status = 0;
    }
    else if (Status == 2) //ending
    {
        Frame = EndAt;
        FrameElapsed = 0;
        Status = 3; //ended

        CompleteCallback.Execute();
    }
    else
    {
        if (Frame == End)
        {
            if (Times > 0)
            {
                Times--;
                if (Times == 0)
                    Status = 2; //ending
                else
                    Status = 1; //new loop
            }
            else if (Start != 0)
                Status = 1; //new loop
        }
    }

    DrawFrame();
}
