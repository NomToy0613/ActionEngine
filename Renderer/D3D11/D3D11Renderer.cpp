#include "D3D11Renderer.h"

D3D11Renderer::D3D11Renderer(int width, int height)
    : width(width), height(height)
{}

bool D3D11Renderer::Initialize(HWND hWnd)
{
    if (!context.Initialize(hWnd, width, height)) return false;
    if (!context.InitTriangle()) return false;   // 仮:三角形テスト用
    return true;
}

void D3D11Renderer::SetWorld(const Matrix4x4& m)      { matWorld = m; }
void D3D11Renderer::SetView(const Matrix4x4& m)       { matView = m; }
void D3D11Renderer::SetProjection(const Matrix4x4& m) { matProj = m; }
void D3D11Renderer::SetEye(const Vector3& e)          { eye = e; }

void D3D11Renderer::BeginFrame()
{
    context.BeginFrame(0.1f, 0.2f, 0.4f);
    context.DrawTriangle();   // 仮:インターフェース経由で動くかの確認用
}

void D3D11Renderer::EndFrame()
{
    context.EndFrame();
}

void D3D11Renderer::DrawMesh(const ObjMesh& mesh, const Material& material)
{
    // 次のステップで実装する
    (void)mesh;
    (void)material;
}