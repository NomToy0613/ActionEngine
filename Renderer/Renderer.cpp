#include "Renderer.h"
#include "Culling.h"
#include "Clipping.h"
#include "Texture.h"
#include <vector>
#include <cmath>
using namespace std;

namespace {
    // 立方体のローカル座標頂点データ（UV付き）※ダイスデモ用、変更なし
    vector<Vector3> cubeVertices = {
        Vector3(-0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f),
        Vector3(0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f),
        Vector3(0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f),
        Vector3(-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f),

        Vector3(-0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f),
        Vector3(0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f),
        Vector3(0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f),
        Vector3(-0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 1.0f),

        Vector3(-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f),
        Vector3(-0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f),
        Vector3(-0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f),
        Vector3(-0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 1.0f),

        Vector3(0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f),
        Vector3(0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f),
        Vector3(0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f),
        Vector3(0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f),

        Vector3(-0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f),
        Vector3(-0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f),
        Vector3(0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f),
        Vector3(0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 1.0f),

        Vector3(-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f),
        Vector3(0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f),
        Vector3(0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f),
        Vector3(-0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f),
    };

    int triangles[12][3] = {
        {0,1,2},   {0,2,3},
        {4,5,6},   {4,6,7},
        {8,9,10},  {8,10,11},
        {12,13,14},{12,14,15},
        {16,17,18},{16,18,19},
        {20,21,22},{20,22,23}
    };

    COLORREF faceColors[12] = {
        RGB(255,   0,   0), RGB(255,   0,   0),
        RGB(0, 255,   0), RGB(0, 255,   0),
        RGB(0,   0, 255), RGB(0,   0, 255),
        RGB(255, 255,   0), RGB(255, 255,   0),
        RGB(255,   0, 255), RGB(255,   0, 255),
        RGB(0, 255, 255), RGB(0, 255, 255)
    };

    const Vector3 g_lightDir = Vector3(0.4f, -0.6f, 0.5f, 0.0f).normalize();
    const float g_ambient = 0.25f;

    ScreenPoint ToScreenPoint(const Vector3& clipVert, int screenWidth, int screenHeight)
    {
        Vector3 ndc = clipVert;
        float invW = (fabs(clipVert.w) > 1e-6f) ? 1.0f / clipVert.w : 1.0f;

        if (fabs(ndc.w) > 1e-6f) {
            ndc.x /= ndc.w;
            ndc.y /= ndc.w;
            ndc.z /= ndc.w;
        }
        ScreenPoint sp;
        sp.x = (int)((ndc.x + 1.0f) * 0.5f * screenWidth);
        sp.y = (int)((1.0f - ndc.y) * 0.5f * screenHeight);
        sp.z = ndc.z;

        sp.invW = invW;
        sp.uOverW = clipVert.u * invW;
        sp.vOverW = clipVert.v * invW;
        return sp;
    }

    // ---- ダイスキューブ専用の描画 ----
    void DrawTriangle(Rasterizer& rasterizer, int faceIndex,
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

        const Texture* texture = (faceTextures != nullptr) ? faceTextures[faceIndex / 2] : nullptr;

        if (IsBackFace(worldVerts[ia], worldVerts[ib], worldVerts[ic], eye))
            return;

        Vector3 edge1 = worldVerts[ib].sub(worldVerts[ia]);
        Vector3 edge2 = worldVerts[ic].sub(worldVerts[ia]);
        Vector3 faceNormal = edge1.crosspro(edge2).normalize();

        float ndotl = faceNormal.dotpro(g_lightDir.ply(-1.0f));
        if (ndotl < 0.0f) ndotl = 0.0f;
        float lightIntensity = g_ambient + (1.0f - g_ambient) * ndotl;

        if (IsTriviallyRejected(outCodes[ia], outCodes[ib], outCodes[ic]))
            return;

        if (IsTriviallyAcepted(outCodes[ia], outCodes[ib], outCodes[ic])) {
            rasterizer.FillTriangle(
                screenVerts[ia].x, screenVerts[ia].y, screenVerts[ia].z,
                screenVerts[ia].uOverW, screenVerts[ia].vOverW, screenVerts[ia].invW,
                screenVerts[ib].x, screenVerts[ib].y, screenVerts[ib].z,
                screenVerts[ib].uOverW, screenVerts[ib].vOverW, screenVerts[ib].invW,
                screenVerts[ic].x, screenVerts[ic].y, screenVerts[ic].z,
                screenVerts[ic].uOverW, screenVerts[ic].vOverW, screenVerts[ic].invW,
                texture, faceColors[faceIndex], lightIntensity
            );
            return;
        }

        vector<Vector3> clipped = ClippingTriangle(
            clipVerts[ia], clipVerts[ib], clipVerts[ic],
            outCodes[ia], outCodes[ib], outCodes[ic]
        );

        if (clipped.size() < 3) return;

        vector<ScreenPoint> clippedScreen;
        clippedScreen.reserve(clipped.size());
        for (const Vector3& cv : clipped)
            clippedScreen.push_back(ToScreenPoint(cv, rasterizer.GetWidth(), rasterizer.GetHeight()));

        for (size_t k = 1; k + 1 < clippedScreen.size(); k++) {
            rasterizer.FillTriangle(
                clippedScreen[0].x, clippedScreen[0].y, clippedScreen[0].z,
                clippedScreen[0].uOverW, clippedScreen[0].vOverW, clippedScreen[0].invW,
                clippedScreen[k].x, clippedScreen[k].y, clippedScreen[k].z,
                clippedScreen[k].uOverW, clippedScreen[k].vOverW, clippedScreen[k].invW,
                clippedScreen[k + 1].x, clippedScreen[k + 1].y, clippedScreen[k + 1].z,
                clippedScreen[k + 1].uOverW, clippedScreen[k + 1].vOverW, clippedScreen[k + 1].invW,
                texture, faceColors[faceIndex], lightIntensity
            );
        }
    }

    Vector3 VertexToVector3(const Vertex& vert)
    {
        return Vector3(vert.position.x, vert.position.y, vert.position.z, 1.0f, vert.u, vert.v);
    }

    // ---- 任意ObjMesh用の描画 ----
    void DrawMeshTriangle(Rasterizer& rasterizer, int ia, int ib, int ic,
        const std::vector<ScreenPoint>& screenVerts,
        const std::vector<Vector3>& worldVerts,
        const std::vector<Vector3>& clipVerts,
        const std::vector<uint8_t>& outCodes,
        const Vector3& eye,
        const Material& material)
    {
        if (IsBackFace(worldVerts[ia], worldVerts[ib], worldVerts[ic], eye))
            return;

        Vector3 edge1 = worldVerts[ib].sub(worldVerts[ia]);
        Vector3 edge2 = worldVerts[ic].sub(worldVerts[ia]);
        Vector3 faceNormal = edge1.crosspro(edge2).normalize();

        float ndotl = faceNormal.dotpro(g_lightDir.ply(-1.0f));
        if (ndotl < 0.0f) ndotl = 0.0f;
        float lightIntensity = g_ambient + (1.0f - g_ambient) * ndotl;

        if (IsTriviallyRejected(outCodes[ia], outCodes[ib], outCodes[ic]))
            return;

        if (IsTriviallyAcepted(outCodes[ia], outCodes[ib], outCodes[ic])) {
            rasterizer.FillTriangle(
                screenVerts[ia].x, screenVerts[ia].y, screenVerts[ia].z,
                screenVerts[ia].uOverW, screenVerts[ia].vOverW, screenVerts[ia].invW,
                screenVerts[ib].x, screenVerts[ib].y, screenVerts[ib].z,
                screenVerts[ib].uOverW, screenVerts[ib].vOverW, screenVerts[ib].invW,
                screenVerts[ic].x, screenVerts[ic].y, screenVerts[ic].z,
                screenVerts[ic].uOverW, screenVerts[ic].vOverW, screenVerts[ic].invW,
                material.texture, material.fallbackColor, lightIntensity
            );
            return;
        }

        vector<Vector3> clipped = ClippingTriangle(
            clipVerts[ia], clipVerts[ib], clipVerts[ic],
            outCodes[ia], outCodes[ib], outCodes[ic]
        );

        if (clipped.size() < 3) return;

        vector<ScreenPoint> clippedScreen;
        clippedScreen.reserve(clipped.size());
        for (const Vector3& cv : clipped)
            clippedScreen.push_back(ToScreenPoint(cv, rasterizer.GetWidth(), rasterizer.GetHeight()));

        for (size_t k = 1; k + 1 < clippedScreen.size(); k++) {
            rasterizer.FillTriangle(
                clippedScreen[0].x, clippedScreen[0].y, clippedScreen[0].z,
                clippedScreen[0].uOverW, clippedScreen[0].vOverW, clippedScreen[0].invW,
                clippedScreen[k].x, clippedScreen[k].y, clippedScreen[k].z,
                clippedScreen[k].uOverW, clippedScreen[k].vOverW, clippedScreen[k].invW,
                clippedScreen[k + 1].x, clippedScreen[k + 1].y, clippedScreen[k + 1].z,
                clippedScreen[k + 1].uOverW, clippedScreen[k + 1].vOverW, clippedScreen[k + 1].invW,
                material.texture, material.fallbackColor, lightIntensity
            );
        }
    }
} // namespace

Renderer::Renderer(int width, int height)
    : rasterizer(width, height)
{}

void Renderer::SetWorld(const Matrix4x4& m) { matWorld = m; }
void Renderer::SetView(const Matrix4x4& m) { matView = m; }
void Renderer::SetProjection(const Matrix4x4& m) { matProj = m; }
void Renderer::SetEye(const Vector3& e) { eye = e; }

void Renderer::BeginFrame()
{
    rasterizer.Clear();
}

void Renderer::EndFrame(HWND hWnd)
{
    rasterizer.Present(hWnd);
}

void Renderer::DrawMesh(const ObjMesh& mesh, const Material& material)
{
    if (!mesh.IsValid()) return;

    Matrix4x4 matWVP = matWorld.ply(matView).ply(matProj);

    const size_t vertCount = mesh.vertices.size();

    std::vector<ScreenPoint> screenVertices(vertCount);
    std::vector<Vector3>     worldVertices(vertCount);
    std::vector<Vector3>     clipVertices(vertCount);
    std::vector<uint8_t>     outCodes(vertCount);

    for (size_t i = 0; i < vertCount; i++) {
        Vector3 v = VertexToVector3(mesh.vertices[i]);
        worldVertices[i] = matWorld.transform(v);
        clipVertices[i] = matWVP.transform(v);
        outCodes[i] = ComputeOutCode(clipVertices[i]);
        screenVertices[i] = ToScreenPoint(clipVertices[i], rasterizer.GetWidth(), rasterizer.GetHeight());
    }

    const size_t triCount = mesh.indices.size() / 3;
    for (size_t t = 0; t < triCount; t++) {
        int ia = mesh.indices[t * 3 + 0];
        int ib = mesh.indices[t * 3 + 1];
        int ic = mesh.indices[t * 3 + 2];
        DrawMeshTriangle(rasterizer, ia, ib, ic, screenVertices, worldVertices, clipVertices, outCodes, eye, material);
    }
}

void Renderer::DrawDiceFrame(const Texture* const faceTextures[FACE_COUNT])
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
        screenVertices[i] = ToScreenPoint(clipVertices[i], rasterizer.GetWidth(), rasterizer.GetHeight());
    }

    for (int i = 0; i < 12; i++)
        DrawTriangle(rasterizer, i, screenVertices, worldVertices, clipVertices, outCodes, eye, faceTextures);
}