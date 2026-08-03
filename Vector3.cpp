#include "Vector3.h"
#include <cmath>
using namespace std;

//足し算
Vector3 Vector3::add(const Vector3& o) const {
    return Vector3(x + o.x, y + o.y, z + o.z, w + o.w);
}

//引き算
Vector3 Vector3::sub(const Vector3& o) const {
    return Vector3(x - o.x, y - o.y, z - o.z, w - o.w);
}

//スカラー倍
Vector3 Vector3::ply(float a) const {
    return Vector3(x * a, y * a, z * a, w * a);
}

//距離の二乗
float Vector3::lensq() const {
    return x * x + y * y + z * z;
}

//距離計算
float Vector3::length() const {
    return sqrtf(lensq());
}

//正規化
Vector3 Vector3::normalize() const {
    float len = length();

    if (len > 0.0f) {
        float invLen = 1.0f / len;
        return Vector3(x * invLen, y * invLen, z * invLen, 0.0f);
    }

    return Vector3(0.0f, 0.0f, 0.0f, 0.0f);
}

//内積
float Vector3::dotpro(const Vector3& o) const {
    return (x * o.x) + (y * o.y) + (z * o.z);
}

//外積
Vector3 Vector3::crosspro(const Vector3& o) const {
    return Vector3(
        (y * o.z) - (z * o.y),
        (z * o.x) - (x * o.z),
        (x * o.y) - (y * o.x),
        0.0f
    );
}