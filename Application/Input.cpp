#include "Input.h"
#include <windows.h>

namespace {
    // KeyCode -> Win32 仮想キーコードへの変換
    // 未対応のKeyCodeが来た場合は0を返し、呼び出し側でfalse扱いにする
    int ToVK(KeyCode key)
    {
        switch (key) {
        case KeyCode::W:               return 'W';
        case KeyCode::A:               return 'A';
        case KeyCode::S:               return 'S';
        case KeyCode::D:               return 'D';
        case KeyCode::Space:           return VK_SPACE;
        case KeyCode::Escape:          return VK_ESCAPE;
        case KeyCode::LeftMouseButton: return VK_LBUTTON;
        case KeyCode::RightMouseButton:return VK_RBUTTON;
        default:                       return 0;
        }
    }
}

bool Input::GetKey(KeyCode key)
{
    int vk = ToVK(key);
    if (vk == 0) return false;

    // 最上位ビットが立っていれば押下中
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}