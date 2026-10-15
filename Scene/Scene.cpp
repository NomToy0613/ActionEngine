#include "Scene.h"

GameObject* Scene::CreateGameObject(const std::string& name)
{
    objects.push_back(std::make_unique<GameObject>(name));
    return objects.back().get();
}

GameObject* Scene::Find(const std::string& name) const
{
    for(const auto& go : objects){
        if(go->name == name)
            return go.get();
    }

    return nullptr;
}

void Scene::Render(Renderer& renderer, const Matrix4x4& matView, const Matrix4x4& matProj, const Vector3& eye) const
{
    renderer.SetView(matView);
    renderer.SetProjection(matProj);
    renderer.SetEye(eye);

    for (const auto& go : objects) {
        if (!go->hasMeshRenderer || go->meshRenderer.mesh == nullptr) continue;

        renderer.SetWorld(go->transform.world());
        renderer.DrawMesh(*go->meshRenderer.mesh, go->meshRenderer.material);
    }
}