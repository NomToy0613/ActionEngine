#pragma once
#include <windows.h>
#include <cstdint>
#include "Texture.h"
#include "FrameBuffer.h"
#include "DepthBuffer.h"

class Rasterizer {
public:
    Rasterizer(int width, int height);

    int GetWidth()  const { return width; }
    int GetHeight() const { return height; }

    void Clear();                                       // フレーム先頭でzBuffer/pixelBufferを初期化
    void DrawPixel(int x, int y, COLORREF color);
    void DrawLine(int x0, int y0, int x1, int y1, COLORREF color);

    // 三角形を塗りつぶす。texture が非nullかつ有効な場合はテクスチャマッピングを行い、
    // それ以外の場合は fallbackColor で単色塗りつぶしを行う。
    void FillTriangle(int x0, int y0, float z0, float uw0, float vw0, float invW0,
        int x1, int y1, float z1, float uw1, float vw1, float invW1,
        int x2, int y2, float z2, float uw2, float vw2, float invW2,
        const Texture* texture, COLORREF fallbackColor, float lightIntensity = 1.0f);

    void Present(HWND hWnd) const;                      // StretchDIBitsで画面転送

private:
    int width;
    int height;
    FrameBuffer frameBuffer;
    DepthBuffer depthBuffer;

    float Edge(float ax, float ay, float bx, float by, float px, float py);
};