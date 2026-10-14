#pragma once
#include <vector>

class DepthBuffer {
public:
    DepthBuffer(int width, int height);

    void Clear(float value = 999999.0f);

    float Get(int x, int y) const;
    void  Set(int x, int y, float value);

    // z値が既存より手前ならtrueを返しつつ書き込む（FillTriangle内の判定をここに寄せる）
    bool TestAndSet(int x, int y, float z);

    int GetWidth()  const { return width; }
    int GetHeight() const { return height; }

private:
    int width;
    int height;
    std::vector<float> depths;

    int Index(int x, int y) const { return y * width + x; }
};