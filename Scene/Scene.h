#pragma once
#include <vector>
#include <memory>
#include "GameObject.h"
#include "Matrix4x4.h"
#include "Renderer.h"

class Scene {
public:
    std::vector<std::unique_ptr<GameObject>> objects;

    GameObject* CreateGameObject(const std::string& name = "GameObject");
    GameObject* Find(const std::string& name) const;

    // Rendererに現在のView/Projection/Eyeをセットしたうえで、
    // 全オブジェクトを順にDrawMeshする
    void Render(Renderer& renderer, const Matrix4x4& matView, const Matrix4x4& matProj, const Vector3& eye) const;
};