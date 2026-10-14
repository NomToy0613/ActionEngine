#pragma once

// フレーム間の経過時間（deltaTime）を管理するクラス。
// GetTickCount64ベースなので複数フレームまたいでも精度劣化しにくい
class Time {
public:
    void Tick();
    float GetDeltaTime() const { return deltaTime; }

private:
    unsigned long long prevTicks = 0;
    float deltaTime = 0.0f;
    bool  first = true;
};