#pragma once
#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix4x4.h"

class Transform {
public:
    Vector3 posi;    // 位置
    Quaternion rota; // 回転
    Vector3 scale;   // 拡大縮小

    // コンストラクタ
    Transform() :
        posi(0.0f, 0.0f, 0.0f),
        rota(0.0f, 0.0f, 0.0f, 1.0f),
        scale(1.0f, 1.0f, 1.0f) {}

    Matrix4x4 world() const;
};