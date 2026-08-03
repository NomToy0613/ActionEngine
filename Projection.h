#pragma once
#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix4x4.h"

class Projection {
public:
	float fov;    //画角
	float aspect; //アスペクト比
	float nearZ;  //前方クリップ面
	float farZ;   //後方クリップ面

	//コンストラクタ
	Projection() :
		fov(0.785398f),   //画角45度
		aspect(16.0f / 9.0f),
		nearZ(0.1f),
		farZ(100.0f)
	{}

	Matrix4x4 projection() const;
};