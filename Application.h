#pragma once
#include <windows.h>
#include "Window.h"
#include "Time.h"
#include "Renderer.h"

// エンジンの土台。Window/Time/Renderer を内部に持ち、
// メインループ（メッセージポンプ→GameUpdate→描画）の面倒を一手に引き受ける。
// ゲームロジック側（GameUpdate/GameRender）はApplicationの存在を意識せずに書ける
class Application {
public:
    Application(HINSTANCE hInstance, int nCmdShow, int width, int height);

    bool Initialize();
    int  Run();

private:
    HINSTANCE hInstance;
    int nCmdShow;

    Window   window;
    Time     time;
    Renderer renderer;

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};