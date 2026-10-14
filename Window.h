#pragma once
#include <windows.h>
#include <string>

// ウィンドウの生成とメッセージポンプ処理のみを責務とするクラス。
// WndProcは呼び出し側（Application）から関数ポインタとして渡してもらう
class Window {
public:
    Window(int width, int height, const std::wstring& title);

    // ウィンドウクラス登録〜CreateWindow〜表示までを行う
    // 成功時true、失敗時（CreateWindow失敗など）はfalseを返す
    bool Create(HINSTANCE hInstance, int nCmdShow, WNDPROC wndProc);

    // キューにたまっているメッセージを処理する（1フレーム分）
    // WM_QUITを受け取った場合はfalseを返す（メインループの終了合図）
    bool PumpMessages();

    HWND GetHandle() const { return hWnd; }
    int  GetWidth()  const { return width; }
    int  GetHeight() const { return height; }

private:
    int width;
    int height;
    std::wstring title;

    HWND hWnd = nullptr;
    HINSTANCE hInstance = nullptr;

    static constexpr wchar_t kClassName[] = L"ActionEngineWindowClass";
};