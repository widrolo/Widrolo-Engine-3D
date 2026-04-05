#pragma once

_GLOBAL_CEX_ float64 pi = 3.141592653589793238462643;
namespace WEngine {

    class Math
    {
    public:
        static float32 Sin(float32 x);
        static float32 Cos(float32 x);
        static float32 Tan(float32 x);
        static float32 Asin(float32 x);
        static float32 Acos(float32 x);
        static float32 Atan(float32 x);
        static float32 Atan2(float32 x, float32 y);

        static float32 Pow(float32 x, int32 exp);
        static float32 Sqrt(float32 x);
    };
}