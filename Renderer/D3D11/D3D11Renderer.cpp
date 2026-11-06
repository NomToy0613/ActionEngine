#include "D3D11Renderer.h"
#include <vector>
#include <cstring>
#include <cmath>

D3D11Renderer::D3D11Renderer(int width, int height)
    : width(width), height(height)
{}

bool D3D11Renderer::Initialize(HWND hWnd)
{
    if (!context.Initialize(hWnd, width, height)) return false;
    if (!context.InitMeshPipeline()) return false;
    return true;
}

void D3D11Renderer::SetWorld(const Matrix4x4& m)      { matWorld = m; }
void D3D11Renderer::SetView(const Matrix4x4& m)       { matView = m; }
void D3D11Renderer::SetProjection(const Matrix4x4& m) { matProj = m; }
void D3D11Renderer::SetEye(const Vector3& e)          { eye = e; }

void D3D11Renderer::BeginFrame()
{
    context.BeginFrame(0.1f, 0.2f, 0.4f);
}

void D3D11Renderer::EndFrame()
{
    context.EndFrame();
}

void D3D11Renderer::DrawMesh(const ObjMesh& mesh, const Material& material)
{
    if (!mesh.IsValid()) return;

    GpuMesh* gpu = GetOrCreateGpuMesh(mesh);
    if (!gpu) return;

    Matrix4x4 matWVP = matWorld.ply(matView).ply(matProj);

    PerObjectCB cb = {};
    memcpy(cb.world,         matWorld.M, sizeof(cb.world));
    memcpy(cb.worldViewProj, matWVP.M,   sizeof(cb.worldViewProj));

    // 光の向きと環境光(ソフトウェア版と同じ値)
    float lx = 0.4f, ly = -0.6f, lz = 0.5f;
    float len = sqrtf(lx * lx + ly * ly + lz * lz);
    cb.lightDir[0] = lx / len;
    cb.lightDir[1] = ly / len;
    cb.lightDir[2] = lz / len;
    cb.ambient     = 0.25f;

    // COLORREF(0〜255) → 0.0〜1.0
    cb.color[0] = GetRValue(material.fallbackColor) / 255.0f;
    cb.color[1] = GetGValue(material.fallbackColor) / 255.0f;
    cb.color[2] = GetBValue(material.fallbackColor) / 255.0f;

    ID3D11ShaderResourceView* srv = nullptr;
    if (material.texture != nullptr) {
        srv = GetOrCreateTexture(*material.texture);
    }
    cb.useTexture = (srv != nullptr) ? 1.0f : 0.0f;

    context.DrawMesh(gpu->vertexBuffer.Get(), gpu->indexBuffer.Get(), gpu->indexCount, cb, srv);
}

D3D11Renderer::GpuMesh* D3D11Renderer::GetOrCreateGpuMesh(const ObjMesh& mesh)
{
    // 既に作ってあればそれを返す
    auto it = meshCache.find(&mesh);
    if (it != meshCache.end()) return &it->second;

    // Vertex → GpuVertex に詰め替える
    std::vector<GpuVertex> vertices(mesh.vertices.size());
    for (size_t i = 0; i < mesh.vertices.size(); i++) {
        const Vertex& v = mesh.vertices[i];
        vertices[i].position[0] = v.position.x;
        vertices[i].position[1] = v.position.y;
        vertices[i].position[2] = v.position.z;
        vertices[i].uv[0] = v.u;
        vertices[i].uv[1] = v.v;
    }

    std::vector<UINT> indices(mesh.indices.begin(), mesh.indices.end());

    GpuMesh gpu;
    ID3D11Device* device = context.GetDevice();

    D3D11_BUFFER_DESC vbd = {};
    vbd.ByteWidth = static_cast<UINT>(sizeof(GpuVertex) * vertices.size());
    vbd.Usage     = D3D11_USAGE_IMMUTABLE;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vinit = {};
    vinit.pSysMem = vertices.data();
    if (FAILED(device->CreateBuffer(&vbd, &vinit, gpu.vertexBuffer.GetAddressOf()))) return nullptr;

    D3D11_BUFFER_DESC ibd = {};
    ibd.ByteWidth = static_cast<UINT>(sizeof(UINT) * indices.size());
    ibd.Usage     = D3D11_USAGE_IMMUTABLE;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA iinit = {};
    iinit.pSysMem = indices.data();
    if (FAILED(device->CreateBuffer(&ibd, &iinit, gpu.indexBuffer.GetAddressOf()))) return nullptr;

    gpu.indexCount = static_cast<UINT>(indices.size());

    auto result = meshCache.emplace(&mesh, gpu);
    return &result.first->second;
}

ID3D11ShaderResourceView* D3D11Renderer::GetOrCreateTexture(const Texture& texture)
{
    auto it = textureCache.find(&texture);
    if (it != textureCache.end()) return it->second.Get();

    if (!texture.IsValid()) return nullptr;

    // RGB(3バイト)→ RGBA(4バイト)に詰め替える
    const size_t pixelCount = static_cast<size_t>(texture.width) * texture.height;
    std::vector<uint8_t> rgba(pixelCount * 4);
    for (size_t i = 0; i < pixelCount; i++) {
        rgba[i * 4 + 0] = texture.pixels[i * 3 + 0];
        rgba[i * 4 + 1] = texture.pixels[i * 3 + 1];
        rgba[i * 4 + 2] = texture.pixels[i * 3 + 2];
        rgba[i * 4 + 3] = 255;
    }

    D3D11_TEXTURE2D_DESC td = {};
    td.Width            = texture.width;
    td.Height           = texture.height;
    td.MipLevels        = 1;
    td.ArraySize        = 1;
    td.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage            = D3D11_USAGE_IMMUTABLE;
    td.BindFlags        = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA init = {};
    init.pSysMem     = rgba.data();
    init.SysMemPitch = texture.width * 4;

    ID3D11Device* device = context.GetDevice();

    Microsoft::WRL::ComPtr<ID3D11Texture2D> gpuTexture;
    if (FAILED(device->CreateTexture2D(&td, &init, gpuTexture.GetAddressOf()))) return nullptr;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
    if (FAILED(device->CreateShaderResourceView(gpuTexture.Get(), nullptr, srv.GetAddressOf()))) return nullptr;

    textureCache[&texture] = srv;
    return srv.Get();
}