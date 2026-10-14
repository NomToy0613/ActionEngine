#pragma once
#include <windows.h>
#include "Vector3.h"
#include "Matrix4x4.h"
#include "Texture.h"
#include "Mesh.h"
#include "Rasterizer.h"

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

// メッシュ全体に適用するマテリアル
struct Material {
    const Texture* texture = nullptr;
    COLORREF fallbackColor = RGB(200, 200, 200);
};

class Renderer {
public:
    Renderer(int width, int height);

    // 毎フレーム、カメラ・変換行列をセットしてからDrawMeshを呼ぶ運用
    void SetWorld(const Matrix4x4& matWorld);
    void SetView(const Matrix4x4& matView);
    void SetProjection(const Matrix4x4& matProj);
    void SetEye(const Vector3& eye);

    // フレームの開始・終了（内部Rasterizerのクリア／画面転送を代行）
    void BeginFrame();
    void EndFrame(HWND hWnd);

    // 完成条件のメインAPI：これ一つでメッシュが描画できる
    void DrawMesh(const ObjMesh& mesh, const Material& material);

    // 旧：ハードコードキューブ描画（ダイスデモ用に維持）
    void DrawDiceFrame(const Texture* const faceTextures[FACE_COUNT] = nullptr);

    Rasterizer& GetRasterizer() { return rasterizer; }

private:
    Rasterizer rasterizer;

    Matrix4x4 matWorld;
    Matrix4x4 matView;
    Matrix4x4 matProj;
    Vector3   eye;
};