#pragma once

// エンジン利用者が扱うキー識別子。
// Win32のVKコードを直接使わせず抽象化することで、将来的に
// 別プラットフォームへ移植する際もゲームロジック側のコードを変えずに済む
enum class KeyCode {
    W, A, S, D, C,
    Space,
    Escape,
    Tab,
    LeftMouseButton,
    RightMouseButton
};

class Input {
private:
    static float wheelDelta;   // このフレームに回した量(ノッチ数)

public:
    static bool GetKey(KeyCode key);     // key が現在押下中かどうかを返す
    static void AddWheel(float notches); // WndProc から呼んで、量を足す
    static float GetWheel();             // GameUpdate から呼んで、量を読む
    static void ResetWheel();            // フレームの最後に 0 に戻す
};