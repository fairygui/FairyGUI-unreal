#pragma once

#include "SFImage.h"

struct FAIRYGUI_API FMovieClipData : public FGCObject
{
    struct Frame
    {
        UNTexture *Texture;
        float AddDelay;
    };

    TArray<Frame> Frames;
    float Interval;
    float RepeatDelay;
    bool bSwing;

    FMovieClipData();
    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
};

class FAIRYGUI_API SMovieClip : public SFImage
{
public:
    SLATE_BEGIN_ARGS(SMovieClip) :
        _GObject(nullptr)
    {}
    SLATE_ARGUMENT(UGObject*, GObject)
    SLATE_END_ARGS()

        SMovieClip();

    void Construct(const FArguments& InArgs);

    void SetClipData(TSharedPtr<FMovieClipData> InData);
    const TSharedPtr<FMovieClipData>& GetClipData() const { return Data; }

    bool IsPlaying() const { return bPlaying; }
    void SetPlaying(bool bInPlaying);

    int32 GetFrame() const { return Frame; }
    void SetFrame(int32 InFrame);

    float GetTimeScale() const { return TimeScale; }
    void SetTimeScale(float InTimeScale);

    void Advance(float Time);

    //from start to end(-1 means ending) repeat times(0 means infinite loop) when all is over, stopping at endAt(-1 means same value of end)
    void SetPlaySettings(int32 InStart = 0, int32 InEnd = -1, int32 InTimes = 0, int32 InEndAt = -1, const FSimpleDelegate& InCompleteCallback = FSimpleDelegate());

public:

    // SWidget overrides
    virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

protected:
    void DrawFrame();

    TSharedPtr<FMovieClipData> Data;
    int32 Frame;
    bool bPlaying;
    float TimeScale;
    int32 Start;
    int32 End;
    int32 Times;
    int32 EndAt;
    int32 Status; //0-none, 1-next loop, 2-ending, 3-ended
    FSimpleDelegate CompleteCallback;

    float FrameElapsed;
    bool bReversed;
    int32 RepeatedCount;
};