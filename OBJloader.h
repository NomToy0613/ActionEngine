#pragma once
#include "Mesh.h"
#include <string>

// Wavefront OBJ (.obj) ファイルを読み込むローダー
// 対応: v(頂点) / vt(UV) / vn(法線) / f(面、三角形・四角形以上はファン分割)
// 未対応: マテリアル(mtl)、グループ、負数インデックス、自由形式曲線など
namespace ObjLoader
{
    // path のOBJファイルを読み込み、outMesh に格納する
    // 成功時 true、失敗時（ファイルが開けない／頂点データが空など）は false を返す
    bool Load(const std::string& path, ObjMesh& outMesh);
}