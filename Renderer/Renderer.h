#pragma once
#include <windows.h>
#include "Vector3.h"
#include "Matrix4x4.h"
#include "Texture.h"
#include "Mesh.h"

// メッシュ全体に適用するマテリアル(どのバックエンドでも共通)
struct Material {
    const Texture* texture = nullptr;
    COLORREF fallbackColor = RGB(200, 200, 200);
};

// 描画バックエンドの共通インターフェース
class Renderer {
public:
    virtual ~Renderer() = default;

    virtual bool Initialize(HWND hWnd) = 0;

    virtual void SetWorld(const Matrix4x4& matWorld) = 0;
    virtual void SetView(const Matrix4x4& matView) = 0;
    virtual void SetProjection(const Matrix4x4& matProj) = 0;
    virtual void SetEye(const Vector3& eye) = 0;

    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;

    virtual void DrawMesh(const ObjMesh& mesh, const Material& material) = 0;
};