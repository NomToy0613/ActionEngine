#pragma once
#include "Mesh.h"
#include "Renderer.h" // Material定義がここにある

struct MeshRenderer {
    const ObjMesh* mesh = nullptr;
    Material material;
};