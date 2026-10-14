#pragma once

// エンジン利用者が扱うキー識別子。
// Win32のVKコードを直接使わせず抽象化することで、将来的に
// 別プラットフォームへ移植する際もゲームロジック側のコードを変えずに済む
enum class KeyCode {
    W, A, S, D,
    Space,
    Escape,
    LeftMouseButton,
    RightMouseButton
};

class Input {
public:
    // key が現在押下中かどうかを返す
    static bool GetKey(KeyCode key);
};