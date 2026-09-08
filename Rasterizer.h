#pragma once
#include <windows.h>
#include <cstdint>
#include "Texture.h"

class Rasterizer {
public:
    static constexpr int WINDOW_WIDTH = 640;
    static constexpr int WINDOW_HEIGHT = 480;

    static void Clear();                                       // フレーム先頭でzBuffer/pixelBufferを初期化
    static void DrawPixel(int x, int y, COLORREF color);
    static void DrawLine(int x0, int y0, int x1, int y1, COLORREF color);

    // 三角形を塗りつぶす。texture が非nullかつ有効な場合はテクスチャマッピングを行い、
    // それ以外の場合は fallbackColor で単色塗りつぶしを行う。
    // uw0/vw0/invW0 等は「u/w, v/w, 1/w」（パースペクティブコレクト補間用に事前計算した値）
    static void FillTriangle(int x0, int y0, float z0, float uw0, float vw0, float invW0,
        int x1, int y1, float z1, float uw1, float vw1, float invW1,
        int x2, int y2, float z2, float uw2, float vw2, float invW2,
        const Texture* texture, COLORREF fallbackColor);

    static void Present(HWND hWnd);                             // StretchDIBitsで画面転送

private:
    static float   zBuffer[WINDOW_WIDTH][WINDOW_HEIGHT];
    static uint32_t pixelBuffer[WINDOW_WIDTH * WINDOW_HEIGHT];

    static float Edge(float ax, float ay, float bx, float by, float px, float py);
};