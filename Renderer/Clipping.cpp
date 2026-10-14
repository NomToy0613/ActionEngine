#include "Clipping.h"
#include "Vector3.h"
#include <vector>
using namespace std;

uint8_t ComputeOutCode(const Vector3& v)
{
    uint8_t outCode = 0;

    if (v.x < -v.w) outCode |= CLIP_LEFT;
    if (v.x > v.w) outCode |= CLIP_RIGHT;
    if (v.y < -v.w) outCode |= CLIP_BOTTOM;
    if (v.y > v.w) outCode |= CLIP_TOP;
    if (v.z < 0.0f) outCode |= CLIP_NEAR;  // ※Zの範囲が [0, w] の場合
    if (v.z > v.w) outCode |= CLIP_FAR;

    return outCode;
}

bool IsTriviallyRejected(uint8_t code0, uint8_t code1, uint8_t code2) {return (code0 & code1 & code2) != 0;}

bool IsTriviallyAcepted(uint8_t code0, uint8_t code1, uint8_t code2) {return (code0 | code1 | code2) == 0;}

// 頂点vが平面planeの内側にあるかどうか（ComputeOutCodeと符号を合わせてある）
static bool IsInside(const Vector3& v, uint8_t plane)
{
    switch (plane)
    {
    case CLIP_LEFT:   return v.x >= -v.w;
    case CLIP_RIGHT:  return v.x <= v.w;
    case CLIP_BOTTOM: return v.y >= -v.w;
    case CLIP_TOP:    return v.y <= v.w;
    case CLIP_NEAR:   return v.z >= 0.0f;
    case CLIP_FAR:    return v.z <= v.w;
    default:          return true;
    }
}

// 辺(cur -> next)と平面の交点をt補間で求める
static Vector3 IntersectPlane(const Vector3& cur, const Vector3& next, uint8_t plane)
{
    float t = 0.0f;

    switch (plane)
    {
    case CLIP_LEFT:   t = (-cur.w - cur.x) / ((next.x - cur.x) - (next.w - cur.w)); break;
    case CLIP_RIGHT:  t = (cur.w - cur.x) / ((next.x - cur.x) - (next.w - cur.w)); break;
    case CLIP_BOTTOM: t = (-cur.w - cur.y) / ((next.y - cur.y) - (next.w - cur.w)); break;
    case CLIP_TOP:    t = (cur.w - cur.y) / ((next.y - cur.y) - (next.w - cur.w)); break;
    case CLIP_NEAR:   t = (0.0f - cur.z) / (next.z - cur.z); break;
    case CLIP_FAR:    t = (cur.w - cur.z) / ((next.z - cur.z) - (next.w - cur.w)); break;
    }

    return Vector3(
        cur.x + (next.x - cur.x) * t,
        cur.y + (next.y - cur.y) * t,
        cur.z + (next.z - cur.z) * t,
        cur.w + (next.w - cur.w) * t
    );
}

// 1平面に対してポリゴンをクリップする（Sutherland-Hodgmanの1ステップ）
vector<Vector3> ClipAgainstPlane(const vector<Vector3>& poly, uint8_t plane)
{
    vector<Vector3> out;
    if (poly.empty()) return out;

    size_t n = poly.size();
    for (size_t i = 0; i < n; i++)
    {
        const Vector3& cur = poly[i];
        const Vector3& next = poly[(i + 1) % n];

        bool curIn = IsInside(cur, plane);
        bool nextIn = IsInside(next, plane);

        if (curIn) out.push_back(cur);

        if (curIn != nextIn)
            out.push_back(IntersectPlane(cur, next, plane));
    }
    return out;
}

// 三角形を6平面すべてでクリップし、結果のポリゴン（0〜9頂点程度）を返す
vector<Vector3> ClippingTriangle(const Vector3& v0, const Vector3& v1, const Vector3& v2, uint8_t code0, uint8_t code1, uint8_t code2)
{
    vector<Vector3> poly = { v0, v1, v2 };

    const uint8_t planes[6] = { CLIP_NEAR, CLIP_LEFT, CLIP_RIGHT, CLIP_BOTTOM, CLIP_TOP, CLIP_FAR };
    uint8_t combined = code0 | code1 | code2;

    for (uint8_t plane : planes)
    {
        if ((combined & plane) == 0) continue;

        poly = ClipAgainstPlane(poly, plane);
        if (poly.empty()) break;
    }

    return poly;
}