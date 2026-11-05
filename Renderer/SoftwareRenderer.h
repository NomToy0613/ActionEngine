#pragma once
#include <windows.h>
#include "Vector3.h"
#include "Matrix4x4.h"
#include "Texture.h"
#include "Mesh.h"
#include "Rasterizer.h"
#include "Renderer.h"

// スクリーン空間の頂点。z はNDCの深度、uOverW/vOverW/invWは
// パースペクティブコレクト補間のために保持している(u/w, v/w, 1/w)
struct ScreenPoint { int x = 0; int y = 0; float z = 0.0f; float uOverW = 0.0f; float vOverW = 0.0f; float invW = 1.0f; };

// 立方体の面インデックス（前/後/左/右/上/下の順）※ダイスデモ用に維持
enum CubeFace {
    FACE_FRONT = 0,
    FACE_BACK = 1,
    FACE_LEFT = 2,
    FACE_RIGHT = 3,
    FACE_TOP = 4,
    FACE_BOTTOM = 5,
    FACE_COUNT = 6
};



class SoftwareRenderer : public Renderer {
public:
    SoftwareRenderer(int width, int height);

    bool Initialize(HWND hWnd) override;

    void SetWorld(const Matrix4x4& matWorld) override;
    void SetView(const Matrix4x4& matView) override;
    void SetProjection(const Matrix4x4& matProj) override;
    void SetEye(const Vector3& eye) override;

    void BeginFrame() override;
    void EndFrame() override;

    void DrawMesh(const ObjMesh& mesh, const Material& material) override;

    // 旧：ハードコードキューブ描画（ダイスデモ用に維持）
    void DrawDiceFrame(const Texture* const faceTextures[FACE_COUNT] = nullptr);

    Rasterizer& GetRasterizer() { return rasterizer; }

private:
    Rasterizer rasterizer;
    HWND hWnd = nullptr;

    Matrix4x4 matWorld;
    Matrix4x4 matView;
    Matrix4x4 matProj;
    Vector3   eye;
};