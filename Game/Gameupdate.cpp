// Gameupdate.cpp
#include <windows.h>
#include <iostream>
#include <vector>
#include "Vector3.h"
#include "Matrix4x4.h"
#include "Quaternion.h"
#include "Transform.h"
#include "Camera.h"
#include "Projection.h"
#include "Input.h"
#include "Renderer.h"
#include "Mesh.h"
#include "SceneManager.h" 

using namespace std;

Transform  t;
Camera     c;
Projection p;

// Sprint3から引き続き使うテスト用メッシュ・マテリアル（ActionEngine.cppで読み込み・設定される）
extern ObjMesh  g_testMesh;
extern Material g_testMaterial;

void GameUpdate(float deltaTime)
{
    // クォータニオンによるCubeの自動回転 (毎フレーム少しずつ回転)
    float rotationSpeed = 1.0f * deltaTime;
    Quaternion deltaRot = Quaternion::axis(rotationSpeed, Vector3(0.0f, 1.0f, 0.0f));

    // 現在の回転に掛け合わせることで、ジンバルロックなしで滑らかに回り続ける
    for (auto& go : SceneManager::GetActiveScene().objects) {
        go->transform.rota = go->transform.rota.ply(deltaRot);
    }


    // マウスによる視点操作（ピッチ・ヨー）
    float mouseDx = 0.0f;
    float mouseDy = 0.0f;

    static POINT prevMousePos = { 0, 0 };
    static bool isDragging = false;

    if (Input::GetKey(KeyCode::LeftMouseButton))
    {
        POINT currentMousePos;
        GetCursorPos(&currentMousePos);

        if (!isDragging)
        {
            isDragging = true;
            prevMousePos = currentMousePos;
        }
        else
        {
            mouseDx = (float)(currentMousePos.x - prevMousePos.x);
            mouseDy = (float)(currentMousePos.y - prevMousePos.y);

            prevMousePos = currentMousePos;
        }
    }
    else
    {
        isDragging = false;
    }

    float sense = 0.002f; // マウス感度
    c.yaw -= mouseDx * sense;
    c.pitch += mouseDy * sense;
    if (c.pitch > 1.5f) c.pitch = 1.5f;
    if (c.pitch < -1.5f) c.pitch = -1.5f;


    // WASDキーによるカメラの移動
    Vector3 forward = c.target.sub(c.eye).normalize();
    Vector3 right = forward.crosspro(c.up).normalize();
    float speed = 5.0f * deltaTime; // 移動速度（1秒間に5ユニット）
    if (Input::GetKey(KeyCode::W))
        c.eye = c.eye.add(forward.ply(speed));
    if (Input::GetKey(KeyCode::S))
        c.eye = c.eye.sub(forward.ply(speed));
    if (Input::GetKey(KeyCode::D))
        c.eye = c.eye.add(right.ply(speed));
    if (Input::GetKey(KeyCode::A))
        c.eye = c.eye.sub(right.ply(speed));

    c.updateVectors();
}

// Application::Run() から毎フレーム呼ばれる描画処理。
// world/view/proj/eyeの計算とRendererへのセットをここに集約することで、
// Application側はGameRenderを呼ぶだけで済み、エンジンとゲームロジックの境界を保つ
void GameRender(Renderer& renderer)
{
    Matrix4x4 matView = c.view();
    Matrix4x4 matProj = p.projection();

    SceneManager::GetActiveScene().Render(renderer, matView, matProj, c.eye);
}