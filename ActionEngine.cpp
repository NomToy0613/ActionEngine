#include "framework.h"
#include "ActionEngine.h"
#include <iostream>
#include <vector>
#include <fcntl.h>

// 3D数学ライブラリをインクルード
#include "Vector3.h"
#include "Matrix4x4.h"
#include "Quaternion.h"
#include "Transform.h"
#include "Camera.h"
#include "Projection.h"

using namespace std;

#define MAX_LOADSTRING 100

// グローバル変数:
HINSTANCE hInst;
HWND g_hWnd = nullptr;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
constexpr int WINDOW_WIDTH = 640;
constexpr int WINDOW_HEIGHT = 480;

extern Transform  t;
extern Camera     c;
extern Projection p;
extern void GameUpdate(float deltaTime);

// 立方体のローカル座標頂点データ
vector<Vector3> cubeVertices = {
     Vector3(-0.5f, -0.5f, -0.5f), // 0: 左下前
     Vector3(0.5f, -0.5f, -0.5f),  // 1: 右下前
     Vector3(-0.5f,  0.5f, -0.5f), // 2: 左上前
     Vector3(0.5f,  0.5f, -0.5f),  // 3: 右上前
     Vector3(-0.5f, -0.5f,  0.5f), // 4: 左下奥
     Vector3(0.5f, -0.5f,  0.5f),  // 5: 右下奥
     Vector3(-0.5f,  0.5f,  0.5f), // 6: 左上奥
     Vector3(0.5f,  0.5f,  0.5f)   // 7: 右上奥
};

int edges[12][2] =
{
    {0,1},{1,3},{3,2},{2,0},
    {4,5},{5,7},{7,6},{6,4},
    {0,4},{1,5},{2,6},{3,7}
};

int triangles[12][3] =
{
    {0,1,2}, {1,3,2}, // 前
    {4,6,5}, {5,6,7}, // 後

    {0,2,4}, {4,2,6}, // 左
    {1,5,3}, {5,7,3}, // 右

    {2,3,6}, {6,3,7}, // 上
    {0,4,1}, {1,4,5}  // 下
};

float zBuffer[WINDOW_WIDTH][WINDOW_HEIGHT];
uint32_t pixelBuffer[WINDOW_WIDTH * WINDOW_HEIGHT];

// デバッグ用コンソールを開く関数
void OpenConsole() {
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    freopen_s(&fp, "CONIN$", "r", stdin);

    cout.clear();
}

// 関数の宣言を転送
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void DrawPixel(int x, int y, COLORREF color)
{
    if (x >= 0 && x < WINDOW_WIDTH && y >= 0 && y < WINDOW_HEIGHT) {
        // WindowsのCOLORREF(0x00BBGGRR)をDIBのピクセル形式(0x00RRGGBB)に合わせる変換
        uint32_t r = (color & 0x000000FF);
        uint32_t g = (color & 0x0000FF00);
        uint32_t b = (color & 0x00FF0000) >> 16;
        uint32_t convertedColor = (r << 16) | g | b;

        pixelBuffer[y * WINDOW_WIDTH + x] = convertedColor;
    }
}

void DrawLine(HDC hdc, int x0, int y0, int x1, int y1, COLORREF color)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int steps = max(abs(dx), abs(dy));
    if (steps == 0)
    {
        DrawPixel(x0, y0, color);
        return;
    }

    float x = (float)x0;
    float y = (float)y0;
    float xInc = (float)dx / steps;
    float yInc = (float)dy / steps;

    for (int i = 0; i <= steps; i++)
    {
        DrawPixel((int)x, (int)y, color);
        x += xInc;
        y += yInc;
    }
}

float Edge(float ax, float ay, float bx, float by, float px, float py)
{
    return (px - ax) * (by - ay) - (py - ay) * (bx - ax);
}

void FillTriangle(HDC hdc, int x0, int y0, float z0, int x1, int y1, float z1, int x2, int y2, float z2, COLORREF color) {
    int minX = min(x0, min(x1, x2));
    int maxX = max(x0, max(x1, x2));
    int minY = min(y0, min(y1, y2));
    int maxY = max(y0, max(y1, y2));
    float area = Edge(x0, y0, x1, y1, x2, y2);
    if (fabs(area) <= 0.0f) return;

    for (int y = minY; y <= maxY; y++)
    {
        for (int x = minX; x <= maxX; x++)
        {
            float w0 = Edge(x1, y1, x2, y2, x, y);
            float w1 = Edge(x2, y2, x0, y0, x, y);
            float w2 = Edge(x0, y0, x1, y1, x, y);

            if ((w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 <= 0 && w1 <= 0 && w2 <= 0)) {
                float wSum = w0 + w1 + w2;
                if (fabs(wSum) < 0.0001f) continue;
                float z = (w0 * z0 + w1 * z1 + w2 * z2) / wSum;
                if (x < 0 || x >= WINDOW_WIDTH || y < 0 || y >= WINDOW_HEIGHT) continue;
                if (z < zBuffer[x][y])
                {
                    zBuffer[x][y] = z;
                    DrawPixel(x, y, color);
                }
            }
        }
    }
}

// メイン関数
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // コンソールを開く
    OpenConsole();

    // グローバル文字列を初期化
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MY, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // アプリケーション初期化の実行:
    if (!InitInstance(hInstance, nCmdShow)) return FALSE;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY));

    MSG msg = {};
    ULONGLONG prevTime = GetTickCount64();

    struct ScreenPoint {
        int x;
        int y;
        float z;
    };

    //リアルタイムゲームループ
    while (msg.message != WM_QUIT)
    {
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

        GameUpdate(deltaTime);

        for (int x = 0; x < WINDOW_WIDTH; x++)
            for (int y = 0; y < WINDOW_HEIGHT; y++)
                zBuffer[x][y] = 999999.0f;

        memset(pixelBuffer, 0, sizeof(pixelBuffer));

        Matrix4x4 matWorld = t.world();
        Matrix4x4 matView = c.view();
        Matrix4x4 matProj = p.projection();
        Matrix4x4 matWVP = matWorld.ply(matView).ply(matProj);

        COORD coord = { 0, 0 };
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);

        cout << "==================================================\n";
        cout << " 自作3D数学エンジン & リアルタイムパイプライン検証\n";
        cout << " 操作方法: [W][A][S][D] キーを長押しでカメラ移動\n";
        cout << "==================================================\n";
        cout << "deltaTime: " << deltaTime << " 秒\n\n";

        cout << "--- カメラ位置 (Eye) --- \n";
        cout << "X: " << c.eye.x << "  Y: " << c.eye.y << "  Z: " << c.eye.z << "      \n\n";

        cout << "--- クォータニオン回転Cubeのリアルタイム変換頂点座標 --- \n";
        for (size_t i = 0; i < cubeVertices.size(); ++i) {
            Vector3 transformedWVP = matWVP.transform(cubeVertices[i]);

            cout << "頂点[" << i << "]\n";
            cout << "Clip : (" << transformedWVP.x << ", " << transformedWVP.y << ", " << transformedWVP.z << ", " << transformedWVP.w << ")\n";

            Vector3 ndc = transformedWVP;

            if (ndc.w != 0.0f)
            {
                ndc.x /= ndc.w;
                ndc.y /= ndc.w;
                ndc.z /= ndc.w;
            }
            int screenX = (int)((ndc.x + 1.0f) * 0.5f * WINDOW_WIDTH);
            int screenY = (int)((1.0f - ndc.y) * 0.5f * WINDOW_HEIGHT);
            cout << "Screen : (" << screenX << ", " << screenY << ")\n";
            cout << "NDC  : (" << ndc.x << ", " << ndc.y << ", " << ndc.z << ")\n\n";

        }

        ScreenPoint screenVertices[8] = {};

        for (size_t i = 0; i < cubeVertices.size(); i++)
        {
            Vector3 clip = matWVP.transform(cubeVertices[i]);

            if (fabs(clip.w) > 1e-6f)
            {
                clip.x /= clip.w;
                clip.y /= clip.w;
                clip.z /= clip.w;
            }

            screenVertices[i].x = (int)((clip.x + 1.0f) * 0.5f * WINDOW_WIDTH);
            screenVertices[i].y = (int)((1.0f - clip.y) * 0.5f * WINDOW_HEIGHT);
            screenVertices[i].z = clip.z;
        }

        COLORREF faceColors[12] = {
            RGB(255,   0,   0), RGB(255,   0,   0), // 前面：赤
            RGB(0, 255,   0), RGB(0, 255,   0), // 背面：緑
            RGB(0,   0, 255), RGB(0,   0, 255), // 上面：青
            RGB(255, 255,   0), RGB(255, 255,   0), // 下面：黄
            RGB(255,   0, 255), RGB(255,   0, 255), // 左面：紫
            RGB(0, 255, 255), RGB(0, 255, 255)  // 右面：水色
        };

        for (int i = 0; i < 12; i++)
        {
            int a = triangles[i][0];
            int b = triangles[i][1];
            int c = triangles[i][2];
            FillTriangle(
                nullptr,
                screenVertices[a].x, screenVertices[a].y, screenVertices[a].z,
                screenVertices[b].x, screenVertices[b].y, screenVertices[b].z,
                screenVertices[c].x, screenVertices[c].y, screenVertices[c].z,
                faceColors[i]
            );
        }

        HDC hdc = GetDC(g_hWnd);

        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = WINDOW_WIDTH;
        bmi.bmiHeader.biHeight = -WINDOW_HEIGHT;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        StretchDIBits(
            hdc,
            0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
            0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
            pixelBuffer,
            &bmi,
            DIB_RGB_COLORS,
            SRCCOPY
        );

        ReleaseDC(g_hWnd, hdc);

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
        CW_USEDEFAULT, 0, WINDOW_WIDTH, WINDOW_HEIGHT, nullptr, nullptr, hInstance, nullptr);

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