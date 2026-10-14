#pragma once
#include "Vector3.h"
#include <vector>

// OBJ由来の頂点。位置・UV・法線を持つ
// Vector3とは別構造体にしてあるのは、法線が「回転のみ・平行移動なし」で
// 変換されるべき特殊なベクトルであり、Vector3(位置用)の変換行列をそのまま
// 適用すると平行移動成分まで乗ってしまってバグるため
struct Vertex {
    Vector3 position;              // x, y, z（w=1.0のまま使う）
    float u = 0.0f, v = 0.0f;      // テクスチャ座標
    float nx = 0.0f, ny = 0.0f, nz = 1.0f; // 法線（未指定時はZ+をデフォルトに）
};

// OBJファイル1つ分のメッシュデータ（頂点共有・インデックスバッファ方式）
struct ObjMesh {
    std::vector<Vertex> vertices;
    std::vector<int>    indices; // 3個で1三角形（フラット配列）

    bool IsValid() const { return !vertices.empty() && !indices.empty(); }
};