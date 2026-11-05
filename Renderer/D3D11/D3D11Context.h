#pragma once
#include <windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <d3dcompiler.h>

class D3D11Context
{
public:
    bool Initialize(HWND hwnd, int width, int height);
    void BeginFrame(float r, float g, float b);
    void EndFrame();
    bool InitTriangle();   // 三角形テスト用(一時的)
    void DrawTriangle();   // 三角形テスト用(一時的)

private:
    Microsoft::WRL::ComPtr<ID3D11Device>           m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext>    m_context;
    Microsoft::WRL::ComPtr<IDXGISwapChain>         m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11VertexShader>     m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>      m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>      m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>           m_vertexBuffer;

    int m_width  = 0;
    int m_height = 0;
};