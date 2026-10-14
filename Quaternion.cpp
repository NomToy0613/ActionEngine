#include "Vector3.h"
#include "Matrix4x4.h"
#include "Quaternion.h"
#include <cmath>
using namespace std;

//Quaternionの作成
Quaternion Quaternion::axis(float angle, const Vector3& axis) {
    Vector3 normAxis = axis.normalize();

    float halfAngle = angle * 0.5f;
    float s = sinf(halfAngle);
    float c = cosf(halfAngle);

    return Quaternion(
        normAxis.x * s,
        normAxis.y * s,
        normAxis.z * s,
        c
    );
}

//掛け算
Quaternion Quaternion::ply(const Quaternion& o) const {
    return Quaternion(
        w * o.x + x * o.w + y * o.z - z * o.y,
        w * o.y - x * o.z + y * o.w + z * o.x,
        w * o.z + x * o.y - y * o.x + z * o.w,
        w * o.w - x * o.x - y * o.y - z * o.z
    );
}

//球面線形補間
Quaternion Quaternion::slerp(const Quaternion& o, float t) const {
    float dot = x * o.x + y * o.y + z * o.z + w * o.w;

    Quaternion target = o;
    if (dot < 0.0f) {
        dot = -dot;
        target.x = -o.x;
        target.y = -o.y;
        target.z = -o.z;
        target.w = -o.w;
    }

    if (dot > 0.9995f) {
        return Quaternion(
            x + (target.x - x) * t,
            y + (target.y - y) * t,
            z + (target.z - z) * t,
            w + (target.w - w) * t
        );
    }

    float theta_0 = acosf(dot);
    float theta = theta_0 * t;
    float sin_theta = sinf(theta);
    float sin_theta_0 = sinf(theta_0);

    float s0 = cosf(theta) - dot * sin_theta / sin_theta_0;
    float s1 = sin_theta / sin_theta_0;

    return Quaternion(
        (x * s0) + (target.x * s1),
        (y * s0) + (target.y * s1),
        (z * s0) + (target.z * s1),
        (w * s0) + (target.w * s1)
    );
}

//Matrix4x4への変換
Matrix4x4 Quaternion::convert() const {
    Matrix4x4 m;

    float xx = x * x; float yy = y * y; float zz = z * z;
    float xy = x * y; float xz = x * z; float yz = y * z;
    float wx = w * x; float wy = w * y; float wz = w * z;

    m.M[0][0] = 1.0f - 2.0f * (yy + zz);
    m.M[0][1] = 2.0f * (xy + wz);
    m.M[0][2] = 2.0f * (xz - wy);
    m.M[0][3] = 0.0f;

    m.M[1][0] = 2.0f * (xy - wz);
    m.M[1][1] = 1.0f - 2.0f * (xx + zz);
    m.M[1][2] = 2.0f * (yz + wx);
    m.M[1][3] = 0.0f;

    m.M[2][0] = 2.0f * (xz + wy);
    m.M[2][1] = 2.0f * (yz - wx);
    m.M[2][2] = 1.0f - 2.0f * (xx + yy);
    m.M[2][3] = 0.0f;

    m.M[3][0] = 0.0f;
    m.M[3][1] = 0.0f;
    m.M[3][2] = 0.0f;
    m.M[3][3] = 1.0f;

    return m;
}

//ベクトルを回転
Vector3 Quaternion::rotaVec(const Vector3& v) const {
    Vector3 qv(x, y, z);
    Vector3 uv = qv.crosspro(v);
    Vector3 uuv = qv.crosspro(uv);

    return v.add(uv.ply(2.0f * w)).add(uuv.ply(2.0f));
}

//各軸の回転角から作成
Quaternion Quaternion::euler(float pitch, float yaw, float roll) {
    float cp = cosf(pitch * 0.5f);  float sp = sinf(pitch * 0.5f);
    float cy = cosf(yaw * 0.5f);    float sy = sinf(yaw * 0.5f);
    float cr = cosf(roll * 0.5f);   float sr = sinf(roll * 0.5f);

    return Quaternion(
        sp * cy * cr + cp * sy * sr, // X
        cp * sy * cr - sp * cy * sr, // Y
        cp * cy * sr - sp * sy * cr, // Z
        cp * cy * cr + sp * sy * sr  // W
    );
}

//Matrix4x4から作成
Quaternion Quaternion::fromMat(const Matrix4x4& m) {
    float tr = m.M[0][0] + m.M[1][1] + m.M[2][2];
    if (tr > 0.0f) {
        float s = sqrtf(tr + 1.0f) * 2.0f;
        return Quaternion(
            (m.M[1][2] - m.M[2][1]) / s,
            (m.M[2][0] - m.M[0][2]) / s,
            (m.M[0][1] - m.M[1][0]) / s,
            0.25f * s
        );
    }
    else if ((m.M[0][0] > m.M[1][1]) && (m.M[0][0] > m.M[2][2])) {
        float s = sqrtf(1.0f + m.M[0][0] - m.M[1][1] - m.M[2][2]) * 2.0f;
        return Quaternion(
            0.25f * s,
            (m.M[0][1] + m.M[1][0]) / s,
            (m.M[2][0] + m.M[0][2]) / s,
            (m.M[1][2] - m.M[2][1]) / s
        );
    }
    else if (m.M[1][1] > m.M[2][2]) {
        float s = sqrtf(1.0f + m.M[1][1] - m.M[0][0] - m.M[2][2]) * 2.0f;
        return Quaternion(
            (m.M[0][1] + m.M[1][0]) / s,
            0.25f * s,
            (m.M[1][2] + m.M[2][1]) / s,
            (m.M[2][0] - m.M[0][2]) / s
        );
    }
    else {
        float s = sqrtf(1.0f + m.M[2][2] - m.M[0][0] - m.M[1][1]) * 2.0f;
        return Quaternion(
            (m.M[2][0] + m.M[0][2]) / s,
            (m.M[1][2] + m.M[2][1]) / s,
            0.25f * s,
            (m.M[0][1] - m.M[1][0]) / s
        );
    }
}

//正規化
Quaternion Quaternion::normalize() const {
    float len = sqrtf(x * x + y * y + z * z + w * w);
    if (len > 0.0f) {
        float invLen = 1.0f / len;
        return Quaternion(x * invLen, y * invLen, z * invLen, w * invLen);
    }
    return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
}