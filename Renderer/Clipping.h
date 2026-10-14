#pragma once
#include "Vector3.h"
#include <cstdint>
#include <vector>

enum ClipCode
{
    CLIP_LEFT = 1 << 0,
    CLIP_RIGHT = 1 << 1,
    CLIP_BOTTOM = 1 << 2,
    CLIP_TOP = 1 << 3,
    CLIP_NEAR = 1 << 4,
    CLIP_FAR = 1 << 5
};

uint8_t ComputeOutCode(const Vector3& v);

bool IsTriviallyRejected(uint8_t code0, uint8_t code1, uint8_t code2);

bool IsTriviallyAcepted(uint8_t code0, uint8_t code1, uint8_t code2);

std::vector<Vector3> ClipAgainstPlane(const std::vector<Vector3>& poly, uint8_t plane);

std::vector<Vector3> ClippingTriangle(const Vector3& v0, const Vector3& v1, const Vector3& v2, uint8_t code0, uint8_t code1, uint8_t code2);