#pragma once

struct Vector3 {
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 1.0f;

	//コンストラクタ
	Vector3() {}
	Vector3(float _x, float _y, float _z, float _w = 1.0f) : x(_x), y(_y), z(_z), w(_w) {}

	Vector3 add(const Vector3& o) const; //足し算
	Vector3 sub(const Vector3& o) const; //引き算
	Vector3 ply(float a) const;          //スカラー倍

	float lensq()const;         //距離の二乗
	float length()const;        //距離計算
	Vector3 normalize() const;  //正規化

	float dotpro(const Vector3& o) const;     //内積
	Vector3 crosspro(const Vector3& o) const; //外積

};