#include "ObjLoader.h"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <cstdio>

namespace {
    // f行の1トークン("v/vt/vn"や"v//vn"や"v"だけ等)を分解する
    struct FaceIndexTriplet {
        int v = 0;  // 1-based。未指定は0
        int vt = 0;
        int vn = 0;
    };

    FaceIndexTriplet ParseFaceToken(const std::string& token)
    {
        FaceIndexTriplet result;

        // "/"区切りで最大3つに分割（空要素も許容: "v//vn"のように）
        int part = 0;
        size_t start = 0;
        for (size_t i = 0; i <= token.size() && part < 3; i++) {
            if (i == token.size() || token[i] == '/') {
                std::string sub = token.substr(start, i - start);
                if (!sub.empty()) {
                    int val = std::atoi(sub.c_str());
                    if (part == 0) result.v = val;
                    else if (part == 1) result.vt = val;
                    else if (part == 2) result.vn = val;
                }
                start = i + 1;
                part++;
            }
        }
        return result;
    }

    // v/vt/vn の組み合わせをキーにして、既出なら同じ頂点インデックスを再利用する
    struct TripletKey {
        int v, vt, vn;
        bool operator==(const TripletKey& o) const {
            return v == o.v && vt == o.vt && vn == o.vn;
        }
    };
    struct TripletKeyHash {
        size_t operator()(const TripletKey& k) const {
            // 簡易ハッシュ結合
            size_t h = std::hash<int>()(k.v);
            h ^= std::hash<int>()(k.vt) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<int>()(k.vn) + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }
    };
}

bool ObjLoader::Load(const std::string& path, ObjMesh& outMesh)
{
    std::ifstream file(path);
    if (!file) return false;

    std::vector<Vector3> positions; // v
    std::vector<std::pair<float, float>> texcoords; // vt (u, v)
    std::vector<Vector3> normals;   // vn（位置と同じ型を使い回すが意味はベクトル）

    outMesh.vertices.clear();
    outMesh.indices.clear();

    std::unordered_map<TripletKey, int, TripletKeyHash> vertexCache;

    std::string line;
    while (std::getline(file, line))
    {
        // 行末のCRなど除去
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n'))
            line.pop_back();
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string tag;
        iss >> tag;

        if (tag == "v")
        {
            float x, y, z;
            iss >> x >> y >> z;
            positions.emplace_back(x, y, z, 1.0f);
        }
        else if (tag == "vt")
        {
            float u, v;
            iss >> u >> v;
            texcoords.emplace_back(u, v);
        }
        else if (tag == "vn")
        {
            float x, y, z;
            iss >> x >> y >> z;
            normals.emplace_back(x, y, z, 0.0f);
        }
        else if (tag == "f")
        {
            // 面を構成する各トークン（3個以上＝三角形以上のポリゴン）を集める
            std::vector<FaceIndexTriplet> faceVerts;
            std::string token;
            while (iss >> token)
                faceVerts.push_back(ParseFaceToken(token));

            if (faceVerts.size() < 3) continue; // 不正な面はスキップ

            // 各トークンをVertexに解決（キャッシュ利用で頂点共有）
            std::vector<int> polyIndices;
            polyIndices.reserve(faceVerts.size());

            for (const auto& fv : faceVerts)
            {
                if (fv.v == 0) continue; // 頂点インデックス必須

                TripletKey key{ fv.v, fv.vt, fv.vn };
                auto it = vertexCache.find(key);
                if (it != vertexCache.end())
                {
                    polyIndices.push_back(it->second);
                    continue;
                }

                Vertex vert;

                // OBJは1-basedインデックス。負数（相対参照）は未対応
                int vIdx = fv.v - 1;
                if (vIdx >= 0 && vIdx < (int)positions.size())
                    vert.position = positions[vIdx];

                if (fv.vt != 0)
                {
                    int vtIdx = fv.vt - 1;
                    if (vtIdx >= 0 && vtIdx < (int)texcoords.size())
                    {
                        vert.u = texcoords[vtIdx].first;
                        vert.v = texcoords[vtIdx].second;
                    }
                }

                if (fv.vn != 0)
                {
                    int vnIdx = fv.vn - 1;
                    if (vnIdx >= 0 && vnIdx < (int)normals.size())
                    {
                        vert.nx = normals[vnIdx].x;
                        vert.ny = normals[vnIdx].y;
                        vert.nz = normals[vnIdx].z;
                    }
                }

                int newIndex = (int)outMesh.vertices.size();
                outMesh.vertices.push_back(vert);
                vertexCache[key] = newIndex;
                polyIndices.push_back(newIndex);
            }

            if (polyIndices.size() < 3) continue;

            // 4頂点以上（quad等）はファン分割して三角形化
            for (size_t k = 1; k + 1 < polyIndices.size(); k++)
            {
                outMesh.indices.push_back(polyIndices[0]);
                outMesh.indices.push_back(polyIndices[k]);
                outMesh.indices.push_back(polyIndices[k + 1]);
            }
        }
        // それ以外のタグ（g, o, usemtl, mtllib, s, #コメント等）は無視
    }

    return outMesh.IsValid();
}