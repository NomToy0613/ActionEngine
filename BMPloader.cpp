#include "BMPLoader.h"
#include <fstream>
#include <cstdint>
#include <vector>

// BMPファイルのバイナリレイアウトそのままの構造体（アライメントパディングを避けるためpack）
#pragma pack(push, 1)
struct BMPFileHeader
{
    uint16_t bfType;      // 'BM' (0x4D42)
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;   // ピクセルデータ開始位置
};

struct BMPInfoHeader
{
    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;    // 正: ボトムアップ格納 / 負: トップダウン格納
    uint16_t biPlanes;
    uint16_t biBitCount;  // 24 or 32 のみ対応
    uint32_t biCompression; // 0 = BI_RGB（無圧縮）のみ対応
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
};
#pragma pack(pop)

bool BMPLoader::Load(const std::string& path, Texture& outTexture)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;

    BMPFileHeader fileHeader{};
    BMPInfoHeader infoHeader{};

    file.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
    if (!file) return false;

    file.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));
    if (!file) return false;

    if (fileHeader.bfType != 0x4D42) return false; // "BM" シグネチャチェック
    if (infoHeader.biCompression != 0) return false; // 無圧縮のみ対応
    if (infoHeader.biBitCount != 24 && infoHeader.biBitCount != 32) return false;
    if (infoHeader.biWidth <= 0 || infoHeader.biHeight == 0) return false;

    const int width = infoHeader.biWidth;
    const bool topDown = infoHeader.biHeight < 0;
    const int height = topDown ? -infoHeader.biHeight : infoHeader.biHeight;
    const int bytesPerPixel = infoHeader.biBitCount / 8;

    // 各行は4バイト境界にパディングされる
    const int rowSize = ((width * bytesPerPixel + 3) / 4) * 4;

    file.seekg(fileHeader.bfOffBits, std::ios::beg);
    if (!file) return false;

    std::vector<uint8_t> rowBuffer(rowSize);
    std::vector<uint8_t> rgbData(static_cast<size_t>(width) * height * 3);

    for (int y = 0; y < height; y++)
    {
        file.read(reinterpret_cast<char*>(rowBuffer.data()), rowSize);
        if (!file) return false;

        // BMPは通常ボトムアップ格納。出力データは上から下(top-to-bottom)の順に統一する
        const int destRow = topDown ? y : (height - 1 - y);

        for (int x = 0; x < width; x++)
        {
            const uint8_t b = rowBuffer[x * bytesPerPixel + 0];
            const uint8_t g = rowBuffer[x * bytesPerPixel + 1];
            const uint8_t r = rowBuffer[x * bytesPerPixel + 2];
            // 32bitの場合4byte目はアルファ等だが、本エンジンでは未使用のため読み捨てる

            const int destIdx = (destRow * width + x) * 3;
            rgbData[destIdx + 0] = r;
            rgbData[destIdx + 1] = g;
            rgbData[destIdx + 2] = b;
        }
    }

    return outTexture.Load(width, height, std::move(rgbData));
}