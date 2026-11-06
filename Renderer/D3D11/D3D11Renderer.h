#pragma once
#include "Renderer.h"
#include "D3D11Context.h"

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
};