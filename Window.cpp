#include "Window.h"

Window::Window(int w, int h, const std::wstring& t)
    : width(w), height(h), title(t)
{}

bool Window::Create(HINSTANCE hInst, int nCmdShow, WNDPROC wndProc)
{
    hInstance = hInst;

    WNDCLASSEXW wcex = { sizeof(WNDCLASSEXW) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = wndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = kClassName;
    wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

    if (!RegisterClassExW(&wcex)) return false;

    hWnd = CreateWindowW(
        kClassName, title.c_str(), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, width, height,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!hWnd) return false;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return true;
}

bool Window::PumpMessages()
{
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT) return false;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}