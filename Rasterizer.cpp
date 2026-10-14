#include "Rasterizer.h"
#include <cmath>
using namespace std;

Rasterizer::Rasterizer(int w, int h)
    : width(w), height(h), frameBuffer(w, h), depthBuffer(w, h)
{}

void Rasterizer::Clear()
{
    depthBuffer.Clear(999999.0f);
    frameBuffer.Clear(0);
}

void Rasterizer::DrawPixel(int x, int y, COLORREF color)
{
    frameBuffer.SetPixel(x, y, color); // 範囲チェック・DIB変換はFrameBuffer側に集約済み
}

void Rasterizer::DrawLine(int x0, int y0, int x1, int y1, COLORREF color)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int steps = max(abs(dx), abs(dy));
    if (steps == 0) {
        DrawPixel(x0, y0, color);
        return;
    }

    float x = (float)x0;
    float y = (float)y0;
    float xInc = (float)dx / steps;
    float yInc = (float)dy / steps;

    for (int i = 0; i <= steps; i++) {
        DrawPixel((int)x, (int)y, color);
        x += xInc;
        y += yInc;
    }
}

float Rasterizer::Edge(float ax, float ay, float bx, float by, float px, float py)
{
    return (px - ax) * (by - ay) - (py - ay) * (bx - ax);
}

void Rasterizer::FillTriangle(int x0, int y0, float z0, float uw0, float vw0, float invW0,
    int x1, int y1, float z1, float uw1, float vw1, float invW1,
    int x2, int y2, float z2, float uw2, float vw2, float invW2,
    const Texture* texture, COLORREF fallbackColor, float lightIntensity)
{
    int minX = max(0, min(x0, min(x1, x2)));
    int maxX = min(width - 1, max(x0, max(x1, x2)));
    int minY = max(0, min(y0, min(y1, y2)));
    int maxY = min(height - 1, max(y0, max(y1, y2)));

    const float fx0 = static_cast<float>(x0), fy0 = static_cast<float>(y0);
    const float fx1 = static_cast<float>(x1), fy1 = static_cast<float>(y1);
    const float fx2 = static_cast<float>(x2), fy2 = static_cast<float>(y2);

    float area = Edge(fx0, fy0, fx1, fy1, fx2, fy2);
    if (fabs(area) <= 0.0f) return;

    if (lightIntensity < 0.0f) lightIntensity = 0.0f;
    if (lightIntensity > 1.0f) lightIntensity = 1.0f;

    const bool useTexture = (texture != nullptr) && texture->IsValid();

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            const float fx = static_cast<float>(x);
            const float fy = static_cast<float>(y);

            float w0 = Edge(fx1, fy1, fx2, fy2, fx, fy);
            float w1 = Edge(fx2, fy2, fx0, fy0, fx, fy);
            float w2 = Edge(fx0, fy0, fx1, fy1, fx, fy);

            if ((w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 <= 0 && w1 <= 0 && w2 <= 0)) {
                float wSum = w0 + w1 + w2;
                if (fabs(wSum) < 0.0001f) continue;
                float z = (w0 * z0 + w1 * z1 + w2 * z2) / wSum;

                // DepthBuffer::TestAndSet で判定と書き込みを一括処理
                if (depthBuffer.TestAndSet(x, y, z)) {

                    COLORREF color = fallbackColor;

                    if (useTexture) {
                        float interpInvW = (w0 * invW0 + w1 * invW1 + w2 * invW2) / wSum;
                        float interpUOverW = (w0 * uw0 + w1 * uw1 + w2 * uw2) / wSum;
                        float interpVOverW = (w0 * vw0 + w1 * vw1 + w2 * vw2) / wSum;

                        float u = 0.0f, v = 0.0f;
                        if (fabs(interpInvW) > 1e-6f) {
                            u = interpUOverW / interpInvW;
                            v = interpVOverW / interpInvW;
                        }

                        uint8_t r, g, b;
                        texture->Sample(u, v, r, g, b);
                        color = RGB(r, g, b);
                    }

                    uint8_t litR = (uint8_t)(GetRValue(color) * lightIntensity);
                    uint8_t litG = (uint8_t)(GetGValue(color) * lightIntensity);
                    uint8_t litB = (uint8_t)(GetBValue(color) * lightIntensity);
                    color = RGB(litR, litG, litB);

                    DrawPixel(x, y, color);
                }
            }
        }
    }
}

void Rasterizer::Present(HWND hWnd) const
{
    frameBuffer.Present(hWnd);
}