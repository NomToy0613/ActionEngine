#pragma once
#include "Resource.h"

// ウィンドウ／描画バッファの基準サイズ。
// RasterizerがWINDOW_WIDTH/HEIGHTのstatic constexprを持たなくなった（インスタンス化した）ため、
// アプリ全体で共有する定数としてここに置く
constexpr int WINDOW_WIDTH = 640;
constexpr int WINDOW_HEIGHT = 480;