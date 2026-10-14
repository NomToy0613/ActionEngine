#include "Application.h"

// GameUpdate.cpp 側で定義されているゲームロジック本体
extern void GameUpdate(float deltaTime);
extern void GameRender(Renderer& renderer);

Application::Application(HINSTANCE hInst, int cmdShow, int width, int height)
    : hInstance(hInst)
    , nCmdShow(cmdShow)
    , window(width, height, L"ActionEngine")
    , renderer(width, height)
{}

bool Application::Initialize()
{
    return window.Create(hInstance, nCmdShow, WndProc);
}

int Application::Run()
{
    while (window.PumpMessages())
    {
        time.Tick();
        float deltaTime = time.GetDeltaTime();

        GameUpdate(deltaTime);

        renderer.BeginFrame();
        GameRender(renderer);
        renderer.EndFrame(window.GetHandle());

        Sleep(16); // 60FPS目安（既存ロジック踏襲）
    }
    return 0;
}

LRESULT CALLBACK Application::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
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