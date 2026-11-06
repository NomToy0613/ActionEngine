#include "Time.h"
#include <windows.h>

void Time::Tick()
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);

    if (first) {
        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);
        frequency   = freq.QuadPart;
        prevCounter = now.QuadPart;
        first = false;
    }

    deltaTime = static_cast<float>(now.QuadPart - prevCounter) / static_cast<float>(frequency);
    if (deltaTime > 0.1f) deltaTime = 0.1f;   // ウィンドウを掴んで止めた後などの大きすぎる値を抑える
    prevCounter = now.QuadPart;
}