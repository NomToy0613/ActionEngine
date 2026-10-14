#pragma once
#include <windows.h>
#include <vector>
#include <cstdint>

class FrameBuffer {
public:
    FrameBuffer(int width, int height);

    void Clear(uint32_t color = 0);
    void SetPixel(int x, int y, COLORREF color); // COLORREF -> DIB形式への変換もここで持つ
    void Present(HWND hWnd) const;

    int GetWidth()  const { return width; }
    int GetHeight() const { return height; }

private:
    int width;
    int height;
    std::vector<uint32_t> pixels;
};