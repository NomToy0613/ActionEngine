#pragma once
#include "Vector3.h"
#include "Matrix4x4.h"
#include "Texture.h"

// スクリーン空間の頂点。z はNDCの深度、uOverW/vOverW/invWは
// パースペクティブコレクト補間のために保持している(u/w, v/w, 1/w)
struct ScreenPoint { int x = 0; int y = 0; float z = 0.0f; float uOverW = 0.0f; float vOverW = 0.0f; float invW = 1.0f; };

// 立方体の面インデックス（前/後/左/右/上/下の順）
enum CubeFace {
    FACE_FRONT = 0,
    FACE_BACK = 1,
    FACE_LEFT = 2,
    FACE_RIGHT = 3,
    FACE_TOP = 4,
    FACE_BOTTOM = 5,
    FACE_COUNT = 6
};

class Renderer {
public:
    // faceTextures は FACE_FRONT〜FACE_BOTTOM(6面)分のテクスチャポインタ配列。
    // 各要素が nullptr の場合はその面だけ単色フォールバック描画になる。
    static void RenderFrame(const Matrix4x4& matWorld,
        const Matrix4x4& matView,
        const Matrix4x4& matProj,
        const Vector3& eye,
        const Texture* const faceTextures[FACE_COUNT] = nullptr);
};