#pragma once
#include "Texture.h"
#include <string>

// 無圧縮(BI_RGB) 24bit / 32bit の BMP ファイルを読み込むローダー
namespace BMPLoader
{
    // path のBMPファイルを読み込み、outTexture に格納する
    // 成功時 true、失敗時（ファイルが開けない／非対応フォーマットなど）は false を返す
    bool Load(const std::string& path, Texture& outTexture);
}