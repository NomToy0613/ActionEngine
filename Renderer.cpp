#include "Renderer.h"
#include "Rasterizer.h"
#include "Culling.h"
#include "Clipping.h"
#include "Texture.h"
#include <vector>
#include <cmath>
using namespace std;

namespace {
    // 立方体のローカル座標頂点データ（UV付き）
    //
    // 面ごとに頂点を共有せず4頂点ずつ用意している。
    // 立方体の各頂点は3つの面で共有されるが、面ごとに異なるUVを貼りたいため、
    // 位置は重複してもよいので「頂点＝位置＋UV」の組として1面につき4個を持たせる。
    // 各面は (u,v) = (0,0),(1,0),(1,1),(0,1) の順で、頂点の巻き順は
    // 外向き法線になるように統一している（裏面カリングと矛盾しないように）。
    vector<Vector3> cubeVertices = {
        // 前面 (Z-) index 0-3
        Vector3(-0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f),
        Vector3(0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f),
        Vector3(0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f),
        Vector3(-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f),

        // 背面 (Z+) index 4-7
        Vector3(-0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f),
        Vector3(0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f),
        Vector3(0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f),
        Vector3(-0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 1.0f),

        // 左面 (X-) index 8-11
        Vector3(-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f),
        Vector3(-0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f),
        Vector3(-0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f),
        Vector3(-0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 1.0f),

        // 右面 (X+) index 12-15
        Vector3(0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f),
        Vector3(0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f),
        Vector3(0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f),
        Vector3(0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f),

        // 上面 (Y+) index 16-19
        Vector3(-0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f),
        Vector3(-0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f),
        Vector3(0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f),
        Vector3(0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 1.0f),

        // 下面 (Y-) index 20-23
        Vector3(-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f),
        Vector3(0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f),
        Vector3(0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f),
        Vector3(-0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f),
    };

    // 三角形ごとの頂点インデックス。2個ずつで1つの面（quad）を構成する。
    // triangles[i]が属する面は i/2 で求まり、CubeFace列挙子の値と対応する
    // （0,1=front / 2,3=back / 4,5=left / 6,7=right / 8,9=top / 10,11=bottom）
    int triangles[12][3] = {
        {0,1,2},   {0,2,3},    // 前  (FACE_FRONT)
        {4,5,6},   {4,6,7},    // 後  (FACE_BACK)
        {8,9,10},  {8,10,11},  // 左  (FACE_LEFT)
        {12,13,14},{12,14,15}, // 右  (FACE_RIGHT)
        {16,17,18},{16,18,19}, // 上  (FACE_TOP)
        {20,21,22},{20,22,23}  // 下  (FACE_BOTTOM)
    };

    // テクスチャが未設定の場合に使うフォールバック色（面ごと）
    COLORREF faceColors[12] = {
        RGB(255,   0,   0), RGB(255,   0,   0), // 前面：赤
        RGB(0, 255,   0), RGB(0, 255,   0),     // 背面：緑
        RGB(0,   0, 255), RGB(0,   0, 255),     // 左面：青
        RGB(255, 255,   0), RGB(255, 255,   0), // 右面：黄
        RGB(255,   0, 255), RGB(255,   0, 255), // 上面：紫
        RGB(0, 255, 255), RGB(0, 255, 255)      // 下面：水色
    };

    ScreenPoint ToScreenPoint(const Vector3& clipVert)
    {
        Vector3 ndc = clipVert;

        // パースペクティブコレクト補間用に 1/w を先に求めておく
        float invW = (fabs(clipVert.w) > 1e-6f) ? 1.0f / clipVert.w : 1.0f;

        if (fabs(ndc.w) > 1e-6f) {
            ndc.x /= ndc.w;
            ndc.y /= ndc.w;
            ndc.z /= ndc.w;
        }
        ScreenPoint sp;
        sp.x = (int)((ndc.x + 1.0f) * 0.5f * Rasterizer::WINDOW_WIDTH);
        sp.y = (int)((1.0f - ndc.y) * 0.5f * Rasterizer::WINDOW_HEIGHT);
        sp.z = ndc.z;

        // u/w, v/w, 1/w を保持（スクリーン空間で線形補間した後に復元することで
        // パースペクティブコレクトなUV補間ができる）
        sp.invW = invW;
        sp.uOverW = clipVert.u * invW;
        sp.vOverW = clipVert.v * invW;
        return sp;
    }

    void DrawTriangle(int faceIndex,
        const ScreenPoint screenVerts[],
        const Vector3 worldVerts[],
        const Vector3 clipVerts[],
        const uint8_t outCodes[],
        const Vector3& eye,
        const Texture* const faceTextures[FACE_COUNT])
    {
        int ia = triangles[faceIndex][0];
        int ib = triangles[faceIndex][1];
        int ic = triangles[faceIndex][2];

        // faceIndex(0-11) は2個で1つの面(quad)を構成するため、面番号は /2 で求まる
        const Texture* texture = (faceTextures != nullptr) ? faceTextures[faceIndex / 2] : nullptr;

        // 裏面カリング
        if (IsBackFace(worldVerts[ia], worldVerts[ib], worldVerts[ic], eye))
            return;

        // 完全に視錐台外なら破棄
        if (IsTriviallyRejected(outCodes[ia], outCodes[ib], outCodes[ic]))
            return;

        // 完全に視錐台内ならそのまま描画
        if (IsTriviallyAcepted(outCodes[ia], outCodes[ib], outCodes[ic])) {
            Rasterizer::FillTriangle(
                screenVerts[ia].x, screenVerts[ia].y, screenVerts[ia].z,
                screenVerts[ia].uOverW, screenVerts[ia].vOverW, screenVerts[ia].invW,
                screenVerts[ib].x, screenVerts[ib].y, screenVerts[ib].z,
                screenVerts[ib].uOverW, screenVerts[ib].vOverW, screenVerts[ib].invW,
                screenVerts[ic].x, screenVerts[ic].y, screenVerts[ic].z,
                screenVerts[ic].uOverW, screenVerts[ic].vOverW, screenVerts[ic].invW,
                texture, faceColors[faceIndex]
            );
            return;
        }

        // 部分クリップが必要なケース（UVもClippingTriangle内で一緒に補間される）
        vector<Vector3> clipped = ClippingTriangle(
            clipVerts[ia], clipVerts[ib], clipVerts[ic],
            outCodes[ia], outCodes[ib], outCodes[ic]
        );

        if (clipped.size() < 3) return;

        vector<ScreenPoint> clippedScreen;
        clippedScreen.reserve(clipped.size());
        for (const Vector3& cv : clipped)
            clippedScreen.push_back(ToScreenPoint(cv));

        // ファン分割して描画
        for (size_t k = 1; k + 1 < clippedScreen.size(); k++) {
            Rasterizer::FillTriangle(
                clippedScreen[0].x, clippedScreen[0].y, clippedScreen[0].z,
                clippedScreen[0].uOverW, clippedScreen[0].vOverW, clippedScreen[0].invW,
                clippedScreen[k].x, clippedScreen[k].y, clippedScreen[k].z,
                clippedScreen[k].uOverW, clippedScreen[k].vOverW, clippedScreen[k].invW,
                clippedScreen[k + 1].x, clippedScreen[k + 1].y, clippedScreen[k + 1].z,
                clippedScreen[k + 1].uOverW, clippedScreen[k + 1].vOverW, clippedScreen[k + 1].invW,
                texture, faceColors[faceIndex]
            );
        }
    }
}

void Renderer::RenderFrame(const Matrix4x4& matWorld,
    const Matrix4x4& matView,
    const Matrix4x4& matProj,
    const Vector3& eye,
    const Texture* const faceTextures[FACE_COUNT])
{
    Matrix4x4 matWVP = matWorld.ply(matView).ply(matProj);

    const size_t vertCount = cubeVertices.size(); // 24

    ScreenPoint screenVertices[24] = {};
    Vector3 worldVertices[24] = {};
    Vector3 clipVertices[24] = {};
    uint8_t outCodes[24] = {};

    for (size_t i = 0; i < vertCount; i++)
        worldVertices[i] = matWorld.transform(cubeVertices[i]);

    for (size_t i = 0; i < vertCount; i++) {
        clipVertices[i] = matWVP.transform(cubeVertices[i]);
        outCodes[i] = ComputeOutCode(clipVertices[i]);
        screenVertices[i] = ToScreenPoint(clipVertices[i]);
    }

    for (int i = 0; i < 12; i++)
        DrawTriangle(i, screenVertices, worldVertices, clipVertices, outCodes, eye, faceTextures);
}