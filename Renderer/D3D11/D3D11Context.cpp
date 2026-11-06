#include "D3D11Context.h"
#include <cstdio>
#include <cstring>

struct TriangleVertex
{
    float position[3];
    float color[3];
};

static bool CompileShader(const wchar_t* path, const char* entry,
                          const char* target, ID3DBlob** blob)
{
    Microsoft::WRL::ComPtr<ID3DBlob> errors;
    HRESULT hr = D3DCompileFromFile(
        path, nullptr, nullptr, entry, target,
        D3DCOMPILE_ENABLE_STRICTNESS, 0,
        blob, errors.GetAddressOf());

    if (FAILED(hr)) {
        if (errors) {
            printf("%s\n", static_cast<const char*>(errors->GetBufferPointer()));
        } else {
            printf("Shader file not found: %ls\n", path);
        }
        return false;
    }
    return true;
}


bool D3D11Context::Initialize(HWND hwnd, int width, int height)
{
    m_width  = width;
    m_height = height;

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

    // 4. 深度バッファを作る
    D3D11_TEXTURE2D_DESC dd = {};
    dd.Width            = width;
    dd.Height           = height;
    dd.MipLevels        = 1;
    dd.ArraySize        = 1;
    dd.Format           = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dd.SampleDesc.Count = 1;
    dd.Usage            = D3D11_USAGE_DEFAULT;
    dd.BindFlags        = D3D11_BIND_DEPTH_STENCIL;

    hr = m_device->CreateTexture2D(&dd, nullptr, m_depthBuffer.GetAddressOf());
    if (FAILED(hr)) return false;

    hr = m_device->CreateDepthStencilView(m_depthBuffer.Get(), nullptr, m_depthView.GetAddressOf());
    if (FAILED(hr)) return false;

    return true;  
}

void D3D11Context::BeginFrame(float r, float g, float b)
{
    const float color[4] = { r, g, b, 1.0f };
    m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthView.Get());
    m_context->ClearRenderTargetView(m_renderTargetView.Get(), color);
    m_context->ClearDepthStencilView(m_depthView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void D3D11Context::EndFrame()
{
    m_swapChain->Present(1, 0);
}

bool D3D11Context::InitTriangle()
{
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> psBlob;

    if (!CompileShader(L"Shaders/Triangle.hlsl", "VSMain", "vs_5_0", vsBlob.GetAddressOf())) return false;
    if (!CompileShader(L"Shaders/Triangle.hlsl", "PSMain", "ps_5_0", psBlob.GetAddressOf())) return false;

    HRESULT hr = m_device->CreateVertexShader(
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
        nullptr, m_vertexShader.GetAddressOf());
    if (FAILED(hr)) return false;

    hr = m_device->CreatePixelShader(
        psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
        nullptr, m_pixelShader.GetAddressOf());
    if (FAILED(hr)) return false;

    // インプットレイアウト: 頂点データの並びをシェーダに教える
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = m_device->CreateInputLayout(
        layout, 2,
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
        m_inputLayout.GetAddressOf());
    if (FAILED(hr)) return false;

    // 頂点データ(時計回り)
    TriangleVertex vertices[] =
    {
        { {  0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },  // 上:赤
        { {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },  // 右下:緑
        { { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } },  // 左下:青
    };

    D3D11_BUFFER_DESC bd = {};
    bd.ByteWidth = sizeof(vertices);
    bd.Usage     = D3D11_USAGE_IMMUTABLE;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices;

    hr = m_device->CreateBuffer(&bd, &initData, m_vertexBuffer.GetAddressOf());
    if (FAILED(hr)) return false;

    return true;
}

void D3D11Context::DrawTriangle()
{
    // RS: ビューポート(NDC → ウィンドウ座標)
    D3D11_VIEWPORT vp = {};
    vp.Width    = static_cast<float>(m_width);
    vp.Height   = static_cast<float>(m_height);
    vp.MaxDepth = 1.0f;
    m_context->RSSetViewports(1, &vp);

    // IA: 頂点データの入力
    UINT stride = sizeof(TriangleVertex);
    UINT offset = 0;
    m_context->IASetInputLayout(m_inputLayout.Get());
    m_context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // VS / PS: シェーダの設定
    m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    // 描画
    m_context->Draw(3, 0);
}

bool D3D11Context::InitMeshPipeline()
{
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> psBlob;

    if (!CompileShader(L"Shaders/Mesh.hlsl", "VSMain", "vs_5_0", vsBlob.GetAddressOf())) return false;
    if (!CompileShader(L"Shaders/Mesh.hlsl", "PSMain", "ps_5_0", psBlob.GetAddressOf())) return false;

    HRESULT hr = m_device->CreateVertexShader(
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_meshVS.GetAddressOf());
    if (FAILED(hr)) return false;

    hr = m_device->CreatePixelShader(
        psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, m_meshPS.GetAddressOf());
    if (FAILED(hr)) return false;

    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    hr = m_device->CreateInputLayout(
        layout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), m_meshLayout.GetAddressOf());
    if (FAILED(hr)) return false;

    // 定数バッファ(毎回の描画でCPUから書き換える)
    D3D11_BUFFER_DESC cbd = {};
    cbd.ByteWidth      = sizeof(PerObjectCB);
    cbd.Usage          = D3D11_USAGE_DYNAMIC;
    cbd.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = m_device->CreateBuffer(&cbd, nullptr, m_perObjectCB.GetAddressOf());
    if (FAILED(hr)) return false;

    D3D11_SAMPLER_DESC sd = {};
    sd.Filter   = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.MaxLOD   = D3D11_FLOAT32_MAX;

    hr = m_device->CreateSamplerState(&sd, m_pointSampler.GetAddressOf());
    return true;
}

void D3D11Context::DrawMesh(ID3D11Buffer* vertexBuffer, ID3D11Buffer* indexBuffer,
                            UINT indexCount, const PerObjectCB& cb,
                            ID3D11ShaderResourceView* texture)
{
    // 定数バッファを書き換える
    D3D11_MAPPED_SUBRESOURCE mapped;
    if (FAILED(m_context->Map(m_perObjectCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) return;
    memcpy(mapped.pData, &cb, sizeof(PerObjectCB));
    m_context->Unmap(m_perObjectCB.Get(), 0);

    // RS: ビューポート
    D3D11_VIEWPORT vp = {};
    vp.Width    = static_cast<float>(m_width);
    vp.Height   = static_cast<float>(m_height);
    vp.MaxDepth = 1.0f;
    m_context->RSSetViewports(1, &vp);

    // IA: 頂点とインデックス
    UINT stride = sizeof(GpuVertex);
    UINT offset = 0;
    m_context->IASetInputLayout(m_meshLayout.Get());
    m_context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    m_context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // VS / PS: シェーダと定数バッファ
    m_context->VSSetShader(m_meshVS.Get(), nullptr, 0);
    m_context->VSSetConstantBuffers(0, 1, m_perObjectCB.GetAddressOf());
    m_context->PSSetShader(m_meshPS.Get(), nullptr, 0);
    m_context->PSSetConstantBuffers(0, 1, m_perObjectCB.GetAddressOf());
    m_context->PSSetShaderResources(0, 1, &texture);
    m_context->PSSetSamplers(0, 1, m_pointSampler.GetAddressOf());

    m_context->DrawIndexed(indexCount, 0, 0);
}