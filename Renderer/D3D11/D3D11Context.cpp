#include "D3D11Context.h"

bool D3D11Context::Initialize(HWND hwnd, int width, int height)
{
    // 1. スワップチェーンの設定
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount       = 1;
    scd.BufferDesc.Width  = width;
    scd.BufferDesc.Height = height;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage       = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow      = hwnd;
    scd.SampleDesc.Count  = 1;
    scd.Windowed          = TRUE;
    scd.SwapEffect        = DXGI_SWAP_EFFECT_DISCARD;

    // 2. Device / DeviceContext / SwapChain をまとめて生成
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,                    // アダプタ(nullptr = 既定のGPU)
        D3D_DRIVER_TYPE_HARDWARE,   // GPUで動かす
        nullptr,                    // ソフトウェアラスタライザDLL(使わない)
        0,                          // 作成フラグ
        nullptr, 0,                 // 機能レベルの候補(nullptr = 既定のリスト)
        D3D11_SDK_VERSION,
        &scd,
        m_swapChain.GetAddressOf(),
        m_device.GetAddressOf(),
        nullptr,                    // 実際に選ばれた機能レベル(今は不要)
        m_context.GetAddressOf());

    if (FAILED(hr)) return false;

    // 3. バックバッファを取り出して RenderTargetView を作る
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    hr = m_swapChain->GetBuffer(
        0,
        __uuidof(ID3D11Texture2D),
        reinterpret_cast<void**>(backBuffer.GetAddressOf()));
    if (FAILED(hr)) return false;

    hr = m_device->CreateRenderTargetView(
        backBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
    if (FAILED(hr)) return false;

    return true;  
}

void D3D11Context::BeginFrame(float r, float g, float b)
{
    const float color[4] = { r, g, b, 1.0f };
    m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);
    m_context->ClearRenderTargetView(m_renderTargetView.Get(), color);
}

void D3D11Context::EndFrame()
{
    m_swapChain->Present(1, 0);
}