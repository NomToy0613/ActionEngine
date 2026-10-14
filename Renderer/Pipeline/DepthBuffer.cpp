#include "DepthBuffer.h"

DepthBuffer::DepthBuffer(int w, int h)
    : width(w), height(h), depths(static_cast<size_t>(w)* h, 999999.0f)
{}

void DepthBuffer::Clear(float value)
{
    std::fill(depths.begin(), depths.end(), value);
}

float DepthBuffer::Get(int x, int y) const
{
    return depths[Index(x, y)];
}

void DepthBuffer::Set(int x, int y, float value)
{
    depths[Index(x, y)] = value;
}

bool DepthBuffer::TestAndSet(int x, int y, float z)
{
    int idx = Index(x, y);
    if (z < depths[idx]) {
        depths[idx] = z;
        return true;
    }
    return false;
}