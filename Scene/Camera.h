#pragma once
#include "Vector3.h"
#include "Matrix4x4.h"

enum class CameraMode {
    FirstPerson,   // 一人称
    ThirdPerson,   // 三人称
    Overhead       // 俯瞰
};

class Camera {
public:
	Vector3 eye;    //カメラの位置
	Vector3 target; //注視点
	Vector3 up;     //上方向ベクトル
	float yaw = 0.0f;   // 水平回転角（マウス用）
	float pitch = -0.3f; // 垂直回転角（マウス用）
	CameraMode mode = CameraMode::ThirdPerson;
	float distance = 6.0f; // 三人称・俯瞰でのターゲットからの距離

	//コンストラクタ
	Camera() :
		eye(0.0f, 3.0f, -8.0f),
		target(0.0f, 0.0f, 0.0f),
		up(0.0f, 1.0f, 0.0f)
	{}

	void update(const Vector3& pivot);
	Matrix4x4 view() const;
	void updateFirstPerson();
	void updateThirdPerson(const Vector3& pivot);
	void updateOverhead();
}; 
