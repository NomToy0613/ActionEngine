#pragma once
#include <windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <d3dcompiler.h>

// メッシュ描画用の頂点(Mesh.hlsl の VSInput と同じ並び)
struct GpuVertex
{
    float position[3];
    float uv[2];
};

// Mesh.hlsl の cbuffer PerObject と同じ並び(16バイト単位)
struct PerObjectCB
{
    float world[16];
    float worldViewProj[16];
    float lightDir[3];
    float ambient;
    float color[3];
    float useTexture;
};

class D3D11Context
{
public:
    bool Initialize(HWND hwnd, int width, int height);
    void BeginFrame(float r, float g, float b);
    void EndFrame();
    bool InitMeshPipeline();

    ID3D11Device* GetDevice() const { return m_device.Get(); }

    void DrawMesh(ID3D11Buffer* m_vertexBuffer, ID3D11Buffer* indexBuffer,
                  UINT indexCount, const PerObjectCB& cb,
                  ID3D11ShaderResourceView* texture);

private:
    Microsoft::WRL::ComPtr<ID3D11Device>           m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext>    m_context;
    Microsoft::WRL::ComPtr<IDXGISwapChain>         m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11Texture2D>        m_depthBuffer;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthView;
    Microsoft::WRL::ComPtr<ID3D11VertexShader>     m_meshVS;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>      m_meshPS;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>      m_meshLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>           m_perObjectCB;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pointSampler;

    int m_width  = 0;
    int m_height = 0;
};