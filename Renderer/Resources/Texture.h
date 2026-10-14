#pragma once
#include <vector>
#include <cstdint>

// テクスチャ画像を保持し、UV座標からピクセル色を取得するクラス
class Texture
{
public:
    int width = 0;
    int height = 0;

    // RGB各1byte、3byte/pixel。格納順は左上から右下（top-to-bottom, left-to-right）
    std::vector<uint8_t> pixels;

    bool IsValid() const { return width > 0 && height > 0 && !pixels.empty(); }

    // 幅・高さとRGBデータを受け取ってテクスチャを初期化する
    bool Load(int w, int h, std::vector<uint8_t>&& rgbData);

    // UV座標（0.0〜1.0を基本とするが範囲外は繰り返しラップする）からRGB色を取得する
    // ニアレストネイバー方式でサンプリングする
    void Sample(float u, float v, uint8_t& outR, uint8_t& outG, uint8_t& outB) const;
};