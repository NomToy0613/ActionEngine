#include "Texture.h"
#include <cmath>

bool Texture::Load(int w, int h, std::vector<uint8_t>&& rgbData)
{
    if (w <= 0 || h <= 0) return false;
    if (rgbData.size() < static_cast<size_t>(w) * static_cast<size_t>(h) * 3) return false;

    width = w;
    height = h;
    pixels = std::move(rgbData);
    return true;
}

void Texture::Sample(float u, float v, uint8_t& outR, uint8_t& outG, uint8_t& outB) const
{
    if (!IsValid())
    {
        // テクスチャ未設定時はマゼンタで欠落を分かりやすくする
        outR = 255; outG = 0; outB = 255;
        return;
    }

    // 0.0〜1.0の範囲外は繰り返し（ラップ）させる
    u = u - floorf(u);
    v = v - floorf(v);

    // 画像データは左上原点(Y下向き)で格納しているが、
    // テクスチャ座標は一般的にVが下から上（OpenGL系）で扱うことが多いため反転させる
    int x = static_cast<int>(u * width);
    int y = static_cast<int>((1.0f - v) * height);

    if (x < 0) x = 0;
    if (x >= width) x = width - 1;
    if (y < 0) y = 0;
    if (y >= height) y = height - 1;

    int idx = (y * width + x) * 3;
    outR = pixels[idx + 0];
    outG = pixels[idx + 1];
    outB = pixels[idx + 2];
}