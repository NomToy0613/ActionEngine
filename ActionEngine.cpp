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

// デバッグ用コンソールを開く関数
void OpenConsole()
{
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    freopen_s(&fp, "CONIN$", "r", stdin);
    cout.clear();
}

// サイコロの6面分のテクスチャを読み込む
// dice_1.bmp 〜 dice_6.bmp を実行ファイルと同じフォルダに配置しておくこと
void LoadDiceTextures()
{
    struct FaceFile { CubeFace face; const char* path; };
    const FaceFile files[FACE_COUNT] = {
        { FACE_FRONT,  "dice_1.bmp" },
        { FACE_BACK,   "dice_6.bmp" },
        { FACE_LEFT,   "dice_2.bmp" },
        { FACE_RIGHT,  "dice_5.bmp" },
        { FACE_TOP,    "dice_3.bmp" },
        { FACE_BOTTOM, "dice_4.bmp" },
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

// アセットの読み込みなど、ゲーム側の起動時初期化をまとめる
void GameInitialize()
{
    LoadDiceTextures();
    Scene& scene = SceneManager::GetActiveScene();

    if (!ObjLoader::Load("polar_bear.obj", g_testMesh)) {
        cout << "[警告] polar_bear.obj の読み込みに失敗しました。\n";
    }
    g_testMaterial.texture = nullptr; // テクスチャ無し、フォールバック色で表示
    g_testMaterial.fallbackColor = RGB(240, 240, 245); // 白熊らしい薄い白

    GameObject* bear1 = scene.CreateGameObject("PolarBear");
    bear1->meshRenderer.mesh = &g_testMesh;
    bear1->meshRenderer.material = g_testMaterial;
    bear1->hasMeshRenderer = true;
    bear1->transform.posi = Vector3(-2.0f, 0.0f, 0.0f);

    if (!ObjLoader::Load("penguin_blue.obj", g_penguinMesh)) {
        cout << "[警告] penguin_blue.obj の読み込みに失敗しました。\n";
    }
    g_penguinMaterial.texture = nullptr;
    g_penguinMaterial.fallbackColor = RGB(40, 90, 200); // 青単色

    GameObject* penguin = scene.CreateGameObject("Penguin");
    penguin->meshRenderer.mesh = &g_penguinMesh;
    penguin->meshRenderer.material = g_penguinMaterial;
    penguin->hasMeshRenderer = true;
    penguin->transform.posi = Vector3(4.0f, 0.0f, 0.0f); // 既存オブジェクトと離した位置
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