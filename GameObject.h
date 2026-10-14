#pragma once
#include <string>
#include "Transform.h"
#include "MeshRenderer.h"

class GameObject {
public:
    std::string name;
    Transform transform;
    MeshRenderer meshRenderer;
    bool hasMeshRenderer = false; // 見た目を持たない空オブジェクトも将来許容するため

    explicit GameObject(const std::string& n = "GameObject") : name(n) {}
};