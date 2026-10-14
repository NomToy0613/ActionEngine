#pragma once
#include "Vector3.h"
#include "Matrix4x4.h"

struct Quaternion {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;

    // コンストラクタ
    Quaternion() {}
    Quaternion(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}

    static Quaternion axis(float angle, const Vector3& axis); //Quaternionの作成

    Quaternion ply(const Quaternion& o) const;            //掛け算
    Quaternion slerp(const Quaternion& o, float t) const; //球面線形補間
    Matrix4x4 convert() const;                            //Matrix4x4への変換 

    Vector3 rotaVec(const Vector3& v) const;                     //ベクトルを回転
    static Quaternion euler(float pitch, float yaw, float roll); //各軸の回転角から作成
    static Quaternion fromMat(const Matrix4x4& m);               //Matrix4x4から作成
    Quaternion normalize() const;                                //正規化

};