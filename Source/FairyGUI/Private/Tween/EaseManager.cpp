#include "Tween/EaseManager.h"

static const float _PiOver2 = (PI * 0.5f);
static const float _TwoPi = (PI * 2);

class Bounce
{
public:
    static float EaseIn(float Time, float Duration);
    static float EaseOut(float Time, float Duration);
    static float EaseInOut(float Time, float Duration);
};

float EaseManager::Evaluate(EEaseType EaseType, float Time, float Duration, float OvershootOrAmplitude, float Period)
{
    switch (EaseType)
    {
    case EEaseType::Linear:
        return Time / Duration;
    case EEaseType::SineIn:
        return -FMath::Cos(Time / Duration * _PiOver2) + 1;
    case EEaseType::SineOut:
        return FMath::Sin(Time / Duration * _PiOver2);
    case EEaseType::SineInOut:
        return -0.5f * (FMath::Cos(PI * Time / Duration) - 1);
    case EEaseType::QuadIn:
        Time /= Duration;
        return Time * Time;
    case EEaseType::QuadOut:
        Time /= Duration;
        return -Time * (Time - 2);
    case EEaseType::QuadInOut:
        Time /= Duration * 0.5f;
        if (Time < 1) return 0.5f * Time * Time;
        --Time;
        return -0.5f * (Time * (Time - 2) - 1);
    case EEaseType::CubicIn:
        Time /= Duration;
        return Time * Time * Time;
    case EEaseType::CubicOut:
        Time = Time / Duration - 1;
        return Time * Time * Time + 1;
    case EEaseType::CubicInOut:
        Time /= Duration * 0.5f;
        if (Time < 1) return 0.5f * Time * Time * Time;
        Time -= 2;
        return 0.5f * (Time * Time * Time + 2);
    case EEaseType::QuartIn:
        Time /= Duration;
        return Time * Time * Time * Time;
    case EEaseType::QuartOut:
        Time = Time / Duration - 1;
        return -(Time * Time * Time * Time - 1);
    case EEaseType::QuartInOut:
        Time /= Duration * 0.5f;
        if (Time < 1) return 0.5f * Time * Time * Time * Time;
        Time -= 2;
        return -0.5f * (Time * Time * Time * Time - 2);
    case EEaseType::QuintIn:
        Time /= Duration;
        return Time * Time * Time * Time * Time;
    case EEaseType::QuintOut:
        Time = Time / Duration - 1;
        return (Time * Time * Time * Time * Time + 1);
    case EEaseType::QuintInOut:
        Time /= Duration * 0.5f;
        if (Time < 1) return 0.5f * Time * Time * Time * Time * Time;
        Time -= 2;
        return 0.5f * (Time * Time * Time * Time * Time + 2);
    case EEaseType::ExpoIn:
        return (Time == 0) ? 0 : FMath::Pow(2, 10 * (Time / Duration - 1));
    case EEaseType::ExpoOut:
        if (Time == Duration) return 1;
        return (-FMath::Pow(2, -10 * Time / Duration) + 1);
    case EEaseType::ExpoInOut:
        if (Time == 0) return 0;
        if (Time == Duration) return 1;
        if ((Time /= Duration * 0.5f) < 1) return 0.5f * FMath::Pow(2, 10 * (Time - 1));
        return 0.5f * (-FMath::Pow(2, -10 * --Time) + 2);
    case EEaseType::CircIn:
        Time /= Duration;
        return -(FMath::Sqrt(1 - Time * Time) - 1);
    case EEaseType::CircOut:
        Time = Time / Duration - 1;
        return FMath::Sqrt(1 - Time * Time);
    case EEaseType::CircInOut:
        Time /= Duration * 0.5f;
        if (Time < 1) return -0.5f * (FMath::Sqrt(1 - Time * Time) - 1);
        Time -= 2;
        return 0.5f * (FMath::Sqrt(1 - Time * Time) + 1);
    case EEaseType::ElasticIn:
        float s0;
        if (Time == 0) return 0;
        if ((Time /= Duration) == 1) return 1;
        if (Period == 0) Period = Duration * 0.3f;
        if (OvershootOrAmplitude < 1)
        {
            OvershootOrAmplitude = 1;
            s0 = Period / 4;
        }
        else s0 = Period / _TwoPi * FMath::Asin(1 / OvershootOrAmplitude);
        Time -= 1;
        return -(OvershootOrAmplitude * FMath::Pow(2, 10 * Time) * FMath::Sin((Time * Duration - s0) * _TwoPi / Period));
    case EEaseType::ElasticOut:
        float s1;
        if (Time == 0) return 0;
        if ((Time /= Duration) == 1) return 1;
        if (Period == 0) Period = Duration * 0.3f;
        if (OvershootOrAmplitude < 1)
        {
            OvershootOrAmplitude = 1;
            s1 = Period / 4;
        }
        else s1 = Period / _TwoPi * FMath::Asin(1 / OvershootOrAmplitude);
        return (OvershootOrAmplitude * FMath::Pow(2, -10 * Time) * FMath::Sin((Time * Duration - s1) * _TwoPi / Period) + 1);
    case EEaseType::ElasticInOut:
        float s;
        if (Time == 0) return 0;
        if ((Time /= Duration * 0.5f) == 2) return 1;
        if (Period == 0) Period = Duration * (0.3f * 1.5f);
        if (OvershootOrAmplitude < 1)
        {
            OvershootOrAmplitude = 1;
            s = Period / 4;
        }
        else s = Period / _TwoPi * FMath::Asin(1 / OvershootOrAmplitude);
        if (Time < 1)
        {
            Time -= 1;
            return -0.5f * (OvershootOrAmplitude * FMath::Pow(2, 10 * Time) * FMath::Sin((Time * Duration - s) * _TwoPi / Period));
        }
            
        Time -= 1;
        return OvershootOrAmplitude * FMath::Pow(2, -10 * Time) * FMath::Sin((Time * Duration - s) * _TwoPi / Period) * 0.5f + 1;
    case EEaseType::BackIn:
        Time /= Duration;
        return Time * Time * ((OvershootOrAmplitude + 1) * Time - OvershootOrAmplitude);
    case EEaseType::BackOut:
        Time = Time / Duration - 1;
        return (Time * Time * ((OvershootOrAmplitude + 1) * Time + OvershootOrAmplitude) + 1);
    case EEaseType::BackInOut:
        Time /= Duration * 0.5f;
        OvershootOrAmplitude *= (1.525f);
        if (Time < 1) return 0.5f * (Time * Time * ((OvershootOrAmplitude + 1) * Time - OvershootOrAmplitude));
        Time -= 2;
        return 0.5f * (Time * Time * ((OvershootOrAmplitude + 1) * Time + OvershootOrAmplitude) + 2);
    case EEaseType::BounceIn:
        return Bounce::EaseIn(Time, Duration);
    case EEaseType::BounceOut:
        return Bounce::EaseOut(Time, Duration);
    case EEaseType::BounceInOut:
        return Bounce::EaseInOut(Time, Duration);

    default:
        Time /= Duration;
        return -Time * (Time - 2);
    }
}

float Bounce::EaseIn(float Time, float Duration)
{
    return 1 - EaseOut(Duration - Time, Duration);
}

float Bounce::EaseOut(float Time, float Duration)
{
    Time /= Duration;
    if (Time < (1 / 2.75f))
    {
        return (7.5625f * Time * Time);
    }
    if (Time < (2 / 2.75f))
    {
        Time -= (1.5f / 2.75f);
        return (7.5625f * Time * Time + 0.75f);
    }
    if (Time < (2.5f / 2.75f))
    {
        Time -= (2.25f / 2.75f);
        return (7.5625f * Time * Time + 0.9375f);
    }
    Time -= (2.625f / 2.75f);
    return (7.5625f * Time * Time + 0.984375f);
}

float Bounce::EaseInOut(float Time, float Duration)
{
    if (Time < Duration * 0.5f)
    {
        return EaseIn(Time * 2, Duration) * 0.5f;
    }
    return EaseOut(Time * 2 - Duration, Duration) * 0.5f + 0.5f;
}

