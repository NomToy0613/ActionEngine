#include "Application.h"
#include "Input.h"

// GameUpdate.cpp 側で定義されているゲームロジック本体
extern void GameUpdate(float deltaTime);
extern void GameRender(Renderer& renderer);

Application::Application(HINSTANCE hInst, int cmdShow, int width, int height)
    : hInstance(hInst)
    , nCmdShow(cmdShow)
    , window(width, height, L"ActionEngine")
    , softwareRenderer(width, height)
    , d3d11Renderer(width, height)
{}

bool Application::Initialize()
{
    if (!window.Create(hInstance, nCmdShow, WndProc)) {
        return false;
    }

    if (!softwareRenderer.Initialize(window.GetHandle())) {
        return false;
    }

    if (!d3d11Renderer.Initialize(window.GetHandle())) {
        printf("D3D11 initialization failed\n");
        return false;
    }

    renderer = &d3d11Renderer;

    return true;
}

int Application::Run()
{
    while (window.PumpMessages())
    {
        time.Tick();
        float deltaTime = time.GetDeltaTime();

        // 1秒ごとにFPSをコンソールに出す
        fpsTimer  += deltaTime;
        fpsFrames += 1;
        if (fpsTimer >= 1.0f) {
            printf("FPS: %d (%s)\n", fpsFrames,
                   renderer == &d3d11Renderer ? "D3D11" : "Software");
            fpsTimer  = 0.0f;
            fpsFrames = 0;
        }

        GameUpdate(deltaTime);

        bool tabDown = Input::GetKey(KeyCode::Tab);
        if (tabDown && !prevTabDown) {
            if (renderer == &d3d11Renderer) {
                renderer = &softwareRenderer;
            } else {
                renderer = &d3d11Renderer;
            }
        }
        prevTabDown = tabDown;

        renderer->BeginFrame();
        GameRender(*renderer);
        renderer->EndFrame();

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