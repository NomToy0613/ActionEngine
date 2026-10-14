#include "FrameBuffer.h"
#include <cstring>

FrameBuffer::FrameBuffer(int w, int h)
    : width(w), height(h), pixels(static_cast<size_t>(w)* h, 0)
{}

void FrameBuffer::Clear(uint32_t color)
{
    std::fill(pixels.begin(), pixels.end(), color);
}

void FrameBuffer::SetPixel(int x, int y, COLORREF color)
{
    if (x < 0 || x >= width || y < 0 || y >= height) return;

    // Rasterizer::DrawPixelにあったCOLORREF(0x00BBGGRR)→DIB(0x00RRGGBB)変換をそのまま移設
    uint32_t r = (color & 0x000000FF);
    uint32_t g = (color & 0x0000FF00);
    uint32_t b = (color & 0x00FF0000) >> 16;
    pixels[y * width + x] = (r << 16) | g | b;
}

void FrameBuffer::Present(HWND hWnd) const
{
    HDC hdc = GetDC(hWnd);

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    StretchDIBits(
        hdc,
        0, 0, width, height,
        0, 0, width, height,
        pixels.data(),
        &bmi,
        DIB_RGB_COLORS,
        SRCCOPY
    );

    ReleaseDC(hWnd, hdc);
}