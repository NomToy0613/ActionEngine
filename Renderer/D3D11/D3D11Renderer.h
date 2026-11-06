#pragma once
#include "Renderer.h"
#include "D3D11Context.h"
#include <unordered_map>

class D3D11Renderer : public Renderer {
public:
    D3D11Renderer(int width, int height);

    bool Initialize(HWND hWnd) override;

    void SetWorld(const Matrix4x4& matWorld) override;
    void SetView(const Matrix4x4& matView) override;
    void SetProjection(const Matrix4x4& matProj) override;
    void SetEye(const Vector3& eye) override;

    void BeginFrame() override;
    void EndFrame() override;

    void DrawMesh(const ObjMesh& mesh, const Material& material) override;

private:
    D3D11Context context;

    int width;
    int height;

    Matrix4x4 matWorld;
    Matrix4x4 matView;
    Matrix4x4 matProj;
    Vector3   eye;

    // ObjMeshをGPU用に変換したもの
    struct GpuMesh {
        Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
        UINT indexCount = 0;
    };

    // 一度変換したメッシュを覚えておく(キーは元のObjMeshのアドレス)
    std::unordered_map<const ObjMesh*, GpuMesh> meshCache;
    std::unordered_map<const Texture*, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureCache;

    GpuMesh* GetOrCreateGpuMesh(const ObjMesh& mesh);
    ID3D11ShaderResourceView* GetOrCreateTexture(const Texture& texture);
};