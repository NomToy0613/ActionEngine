#include "Time.h"
#include <windows.h>

void Time::Tick()
{
    unsigned long long cur = GetTickCount64();
    if (first) {
        prevTicks = cur;
        first = false;
    }

    deltaTime = (cur - prevTicks) / 1000.0f;
    if (deltaTime < 0.001f) deltaTime = 0.016f; // 起動直後などの異常値をガード（既存ロジック踏襲）
    prevTicks = cur;
}