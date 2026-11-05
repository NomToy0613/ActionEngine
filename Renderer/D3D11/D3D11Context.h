#pragma once
#include <windows.h>
#include <d3d11.h>
#include <wrl/client.h>

class D3D11Context
{
public:
    bool Initialize(HWND hwnd, int width, int height);
    void BeginFrame(float r, float g, float b);
    void EndFrame();

private:
    Microsoft::WRL::ComPtr<ID3D11Device>           m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext>    m_context;
    Microsoft::WRL::ComPtr<IDXGISwapChain>         m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
};