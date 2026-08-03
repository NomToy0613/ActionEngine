#pragma once
#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix4x4.h"

class Camera {
public:
	Vector3 eye;    //カメラの位置
	Vector3 target; //注視点
	Vector3 up;     //上方向ベクトル
	float yaw = 0.0f;   // 水平回転角（マウス用）
	float pitch = 0.0f; // 垂直回転角（マウス用）

	//コンストラクタ
	Camera() :
		eye(0.0f, 0.0f, -5.0f),
		target(0.0f, 0.0f, 0.0f),
		up(0.0f, 1.0f, 0.0f)
	{}

	void updateVectors();
	Matrix4x4 view() const;
}; 
