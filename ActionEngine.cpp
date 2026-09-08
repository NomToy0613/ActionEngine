#include "framework.h"
#include "ActionEngine.h"
#include <iostream>
#include <fcntl.h>

// 3D数学ライブラリをインクルード
#include "Vector3.h"
#include "Matrix4x4.h"
#include "Quaternion.h"
#include "Transform.h"
#include "Camera.h"
#include "Projection.h"
#include "Rasterizer.h"
#include "Renderer.h"
#include "Texture.h"
#include "BMPLoader.h"

using namespace std;

#define MAX_LOADSTRING 100

// グローバル変数:
HINSTANCE hInst;
HWND g_hWnd = nullptr;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

// Sprint2: サイコロの6面分のテクスチャ（起動時にBMPから読み込む）
// 面の対応: FACE_FRONT=1, FACE_BACK=6, FACE_LEFT=2, FACE_RIGHT=5, FACE_TOP=3, FACE_BOTTOM=4
// （サイコロの一般的なルールに合わせ、向かい合う面の目の合計が7になるよう配置）
Texture g_diceFaceTextures[FACE_COUNT];
const Texture* g_diceFaceTexturePtrs[FACE_COUNT] = {};

extern Transform  t;
extern Camera     c;
extern Projection p;
extern void GameUpdate(float deltaTime);

// デバッグ用コンソールを開く関数
void OpenConsole() {
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

// 関数の宣言を転送
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// メイン関数
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR    lpCmdLine, _In_ int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // コンソールを開く
    OpenConsole();

    // Sprint2: サイコロ6面分のテクスチャを読み込む
    LoadDiceTextures();

    // グローバル文字列を初期化
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MY, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // アプリケーション初期化の実行:
    if (!InitInstance(hInstance, nCmdShow)) return FALSE;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY));

    MSG msg = {};
    ULONGLONG prevTime = GetTickCount64();

    //リアルタイムゲームループ
    while (msg.message != WM_QUIT) {
        // メッセージ処理
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) break;

            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        if (msg.message == WM_QUIT) break;

        // 時間計算
        ULONGLONG curTime = GetTickCount64();
        float deltaTime = (curTime - prevTime) / 1000.0f;
        if (deltaTime < 0.001f) deltaTime = 0.016f;
        prevTime = curTime;

        // ゲームロジックの更新（入力・カメラ・回転など）
        GameUpdate(deltaTime);

        // フレームバッファの初期化
        Rasterizer::Clear();

        // 変換行列の算出
        Matrix4x4 matWorld = t.world();
        Matrix4x4 matView = c.view();
        Matrix4x4 matProj = p.projection();

        COORD coord = { 0, 0 };
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);

        cout << "==================================================\n";
        cout << " 自作3D数学エンジン & リアルタイムパイプライン検証\n";
        cout << " 操作方法: [W][A][S][D] キーを長押しでカメラ移動\n";
        cout << "==================================================\n";
        cout << "deltaTime: " << deltaTime << " 秒\n\n";

        cout << "--- カメラ位置 (Eye) --- \n";
        cout << "X: " << c.eye.x << "  Y: " << c.eye.y << "  Z: " << c.eye.z << "      \n\n";

        // カリング・クリッピング・ラスタライズを含む描画処理は Renderer に委譲
        // 面ごとに異なるダイステクスチャをバインドして描画
        Renderer::RenderFrame(matWorld, matView, matProj, c.eye, g_diceFaceTexturePtrs);

        // フレームバッファをウィンドウへ転送
        Rasterizer::Present(g_hWnd);

        Sleep(16); // 60FPS
    }

    return (int)msg.wParam;
}

// ウィンドウクラスの登録
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = { sizeof(WNDCLASSEXW) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MY));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MY);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

// ウィンドウインスタンスの生成
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;
    g_hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, Rasterizer::WINDOW_WIDTH, Rasterizer::WINDOW_HEIGHT, nullptr, nullptr, hInstance, nullptr);

    if (!g_hWnd) return FALSE;

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    return TRUE;
}

// メッセージハンドラー
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// バージョン情報
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}