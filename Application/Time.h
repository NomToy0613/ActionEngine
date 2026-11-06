#pragma once

// フレーム間の経過時間（deltaTime）を管理するクラス。
// GetTickCount64ベースなので複数フレームまたいでも精度劣化しにくい
class Time {
public:
    void Tick();
    float GetDeltaTime() const { return deltaTime; }

private:
    long long prevCounter = 0;   // 前のフレームのカウンタ値
    long long frequency   = 0;   // 1秒あたりのカウント数
    float deltaTime = 0.0f;
    bool  first = true;
};