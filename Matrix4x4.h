#pragma once
#include "Vector3.h"

struct Matrix4x4 {

	float M[4][4]{};

	Matrix4x4 identity() const;              //単位行列の作成
	Matrix4x4 ply(const Matrix4x4& o) const; //行列の掛け算

	Matrix4x4 translate(float x, float y, float z) const; //平行移動行列
	Matrix4x4 scale(float x, float y, float z) const;     //拡大縮小行列
	Matrix4x4 rotaX(float angle) const;                   //X軸まわりの回転行列
	Matrix4x4 rotaY(float angle) const;                   //Y軸まわりの回転行列
	Matrix4x4 rotaZ(float angle) const;                   //Z軸まわりの回転行列

	Vector3 transform(const Vector3& o) const; //ベクトルの移動・回転

	Matrix4x4 lookat(const Vector3& eye, const Vector3& target, const Vector3& up) const; //ビュー変換行列
	Matrix4x4 perspe(float fov, float aspect, float near, float far) const;               //プロジェクション変換行列
};