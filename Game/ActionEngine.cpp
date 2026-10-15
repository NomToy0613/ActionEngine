#include <windows.h>
#include <iostream>
#include "ActionEngine.h"
#include "Application.h"
#include "Vector3.h"
#include "Renderer.h"
#include "Texture.h"
#include "BMPloader.h"
#include "OBJloader.h"
#include "Mesh.h"
#include "SceneManager.h" 

using namespace std;

// Sprint2: サイコロの6面分のテクスチャ（起動時にBMPから読み込む）
// 面の対応: FACE_FRONT=1, FACE_BACK=6, FACE_LEFT=2, FACE_RIGHT=5, FACE_TOP=3, FACE_BOTTOM=4
Texture g_diceFaceTextures[FACE_COUNT];
const Texture* g_diceFaceTexturePtrs[FACE_COUNT] = {};

// GameUpdate.cpp から参照される、現在描画中のテスト用メッシュ・マテリアル
// 白熊
ObjMesh  g_testMesh;
Material g_testMaterial;
// ペンギン
ObjMesh  g_penguinMesh;
Material g_penguinMaterial;

// 床
ObjMesh g_groundMesh;
Texture  g_groundTexture;
Material g_groundMaterial;

// デバッグ用コンソールを開く関数
void OpenConsole()
{
    AllocConsole();
    SetConsoleOutputCP(CP_UTF8);
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    freopen_s(&fp, "CONIN$", "r", stdin);
    cout.clear();
}

// サイコロの6面分のテクスチャを読み込む
void LoadDiceTextures()
{
    struct FaceFile { CubeFace face; const char* path; };
    const FaceFile files[FACE_COUNT] = {
        { FACE_FRONT,  "Assets/Textures/dice_1.bmp" },
        { FACE_BACK,   "Assets/Textures/dice_6.bmp" },
        { FACE_LEFT,   "Assets/Textures/dice_2.bmp" },
        { FACE_RIGHT,  "Assets/Textures/dice_5.bmp" },
        { FACE_TOP,    "Assets/Textures/dice_3.bmp" },
        { FACE_BOTTOM, "Assets/Textures/dice_4.bmp" },
    };

    for (const auto& f : files) {
        if (!BMPLoader::Load(f.path, g_diceFaceTextures[f.face])) {
            cout << "[警告] " << f.path << " の読み込みに失敗しました。この面は単色フォールバック描画になります。\n";
            g_diceFaceTexturePtrs[f.face] = nullptr;
        }
        else {
            g_diceFaceTexturePtrs[f.face] = &g_diceFaceTextures[f.face];
        }
    }
}

// 床（四角形1枚）のメッシュを作る。uvRepeat でテクスチャの繰り返し回数を決める
ObjMesh MakeGroundMesh(float halfSize, float uvRepeat)
{
    ObjMesh mesh;
    const float h = halfSize;
    const float r = uvRepeat;

    // 頂点0：左下手前
    Vertex v0;
    v0.position = Vector3(-h, 0.0f, -h);
    v0.u = 0.0f;  v0.v = 0.0f;
    v0.nx = 0.0f; v0.ny = 1.0f; v0.nz = 0.0f;
    mesh.vertices.push_back(v0);

    // 頂点1
    Vertex v1;
    v1.position = Vector3(-h, 0.0f, h);
    v1.u = 0.0f;  v1.v = r;
    v1.nx = 0.0f; v1.ny = 1.0f; v1.nz = 0.0f;
    mesh.vertices.push_back(v1);

    // 頂点2
    Vertex v2;
    v2.position = Vector3(h, 0.0f, h);
    v2.u = r;  v2.v = r;
    v2.nx = 0.0f; v2.ny = 1.0f; v2.nz = 0.0f;
    mesh.vertices.push_back(v2);

    // 頂点3
    Vertex v3;
    v3.position = Vector3(h, 0.0f, -h);
    v3.u = r;  v3.v = 0.0f;
    v3.nx = 0.0f; v3.ny = 1.0f; v3.nz = 0.0f;
    mesh.vertices.push_back(v3);

    // 三角形2つ分の番号
    mesh.indices = { 0, 1, 2,  0, 2, 3 };

    return mesh;
}

// アセットの読み込みなど、ゲーム側の起動時初期化をまとめる
void GameInitialize()
{
    LoadDiceTextures();
    Scene& scene = SceneManager::GetActiveScene();

    if (!ObjLoader::Load("Assets/Models/polar_bear.obj", g_testMesh)) {
        cout << "[警告] polar_bear.obj の読み込みに失敗しました。\n";
    }
    g_testMaterial.texture = nullptr; // テクスチャ無し、フォールバック色で表示
    g_testMaterial.fallbackColor = RGB(240, 240, 245); // 白熊らしい薄い白

    GameObject* bear1 = scene.CreateGameObject("PolarBear");
    bear1->meshRenderer.mesh = &g_testMesh;
    bear1->meshRenderer.material = g_testMaterial;
    bear1->hasMeshRenderer = true;
    bear1->transform.posi = Vector3(-2.0f, 0.0f, 0.0f);

    if (!ObjLoader::Load("Assets/Models/penguin_blue.obj", g_penguinMesh)) {
        cout << "[警告] penguin_blue.obj の読み込みに失敗しました。\n";
    }
    g_penguinMaterial.texture = nullptr;
    g_penguinMaterial.fallbackColor = RGB(40, 90, 200); // 青単色

    GameObject* penguin = scene.CreateGameObject("Penguin");
    penguin->meshRenderer.mesh = &g_penguinMesh;
    penguin->meshRenderer.material = g_penguinMaterial;
    penguin->hasMeshRenderer = true;
    penguin->transform.posi = Vector3(4.0f, 1.45f, 0.0f); // 既存オブジェクトと離した位置

    if(!BMPLoader::Load("Assets/Textures/checker.bmp", g_groundTexture)){
        cout << "[警告] checker.bmp の読み込みに失敗しました。\n";
    }
    g_groundMesh = MakeGroundMesh(20.0f, 20.0f);
    g_groundMaterial.texture = &g_groundTexture;

    GameObject* ground = scene.CreateGameObject("Ground");
    ground->meshRenderer.mesh = &g_groundMesh;
    ground->meshRenderer.material = g_groundMaterial;
    ground->hasMeshRenderer = true;
}

// メイン関数
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    OpenConsole();
    GameInitialize();

    Application app(hInstance, nCmdShow, WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!app.Initialize()) {
        return FALSE;
    }

    return app.Run();
}