#include "Matrix4x4.h"
#include <cmath>
using namespace std;

//単位行列の作成
Matrix4x4 Matrix4x4::identity() const {
    return Matrix4x4{
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

//行列の掛け算
Matrix4x4 Matrix4x4::ply(const Matrix4x4& o) const {
    return Matrix4x4{
        M[0][0] * o.M[0][0] + M[0][1] * o.M[1][0] + M[0][2] * o.M[2][0] + M[0][3] * o.M[3][0],
        M[0][0] * o.M[0][1] + M[0][1] * o.M[1][1] + M[0][2] * o.M[2][1] + M[0][3] * o.M[3][1],
        M[0][0] * o.M[0][2] + M[0][1] * o.M[1][2] + M[0][2] * o.M[2][2] + M[0][3] * o.M[3][2],
        M[0][0] * o.M[0][3] + M[0][1] * o.M[1][3] + M[0][2] * o.M[2][3] + M[0][3] * o.M[3][3],

        M[1][0] * o.M[0][0] + M[1][1] * o.M[1][0] + M[1][2] * o.M[2][0] + M[1][3] * o.M[3][0],
        M[1][0] * o.M[0][1] + M[1][1] * o.M[1][1] + M[1][2] * o.M[2][1] + M[1][3] * o.M[3][1],
        M[1][0] * o.M[0][2] + M[1][1] * o.M[1][2] + M[1][2] * o.M[2][2] + M[1][3] * o.M[3][2],
        M[1][0] * o.M[0][3] + M[1][1] * o.M[1][3] + M[1][2] * o.M[2][3] + M[1][3] * o.M[3][3],

        M[2][0] * o.M[0][0] + M[2][1] * o.M[1][0] + M[2][2] * o.M[2][0] + M[2][3] * o.M[3][0],
        M[2][0] * o.M[0][1] + M[2][1] * o.M[1][1] + M[2][2] * o.M[2][1] + M[2][3] * o.M[3][1],
        M[2][0] * o.M[0][2] + M[2][1] * o.M[1][2] + M[2][2] * o.M[2][2] + M[2][3] * o.M[3][2],
        M[2][0] * o.M[0][3] + M[2][1] * o.M[1][3] + M[2][2] * o.M[2][3] + M[2][3] * o.M[3][3],

        M[3][0] * o.M[0][0] + M[3][1] * o.M[1][0] + M[3][2] * o.M[2][0] + M[3][3] * o.M[3][0],
        M[3][0] * o.M[0][1] + M[3][1] * o.M[1][1] + M[3][2] * o.M[2][1] + M[3][3] * o.M[3][1],
        M[3][0] * o.M[0][2] + M[3][1] * o.M[1][2] + M[3][2] * o.M[2][2] + M[3][3] * o.M[3][2],
        M[3][0] * o.M[0][3] + M[3][1] * o.M[1][3] + M[3][2] * o.M[2][3] + M[3][3] * o.M[3][3]
    };
}

//平行移動行列
Matrix4x4 Matrix4x4::translate(float x, float y, float z) const {
    return Matrix4x4{
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
           x,    y,    z, 1.0f
    };
}

//拡大縮小行列
Matrix4x4 Matrix4x4::scale(float x, float y, float z) const {
    return Matrix4x4{
           x, 0.0f, 0.0f, 0.0f,
        0.0f,    y, 0.0f, 0.0f,
        0.0f, 0.0f,    z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

//X軸まわりの回転行列
Matrix4x4 Matrix4x4::rotaX(float angle) const {
    float c = cosf(angle);
    float s = sinf(angle);
    return Matrix4x4{
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f,    c,    s, 0.0f,
        0.0f,   -s,    c, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

//Y軸まわりの回転行列
Matrix4x4 Matrix4x4::rotaY(float angle) const {
    float c = cosf(angle);
    float s = sinf(angle);
    return Matrix4x4{
           c, 0.0f,   -s, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
           s, 0.0f,    c, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

//Z軸まわりの回転行列
Matrix4x4 Matrix4x4::rotaZ(float angle) const {
    float c = cosf(angle);
    float s = sinf(angle);
    return Matrix4x4{
           c,    s, 0.0f, 0.0f,
          -s,    c, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

//ベクトルの移動・回転
Vector3 Matrix4x4::transform(const Vector3& o) const {
    Vector3 result(
        o.x * M[0][0] + o.y * M[1][0] + o.z * M[2][0] + o.w * M[3][0],
        o.x * M[0][1] + o.y * M[1][1] + o.z * M[2][1] + o.w * M[3][1],
        o.x * M[0][2] + o.y * M[1][2] + o.z * M[2][2] + o.w * M[3][2],
        o.x * M[0][3] + o.y * M[1][3] + o.z * M[2][3] + o.w * M[3][3]
    );

    // UVは空間変換の対象ではないため、そのまま引き継ぐ
    result.u = o.u;
    result.v = o.v;

    return result;
}

//ビュー変換行列
Matrix4x4 Matrix4x4::lookat(const Vector3& eye, const Vector3& target, const Vector3& up) const {
    Vector3 zaxis = target.sub(eye).normalize();
    Vector3 xaxis = up.crosspro(zaxis).normalize();
    Vector3 yaxis = zaxis.crosspro(xaxis);

    return Matrix4x4{
        xaxis.x, yaxis.x, zaxis.x, 0.0f,
        xaxis.y, yaxis.y, zaxis.y, 0.0f,
        xaxis.z, yaxis.z, zaxis.z, 0.0f,
        -xaxis.dotpro(eye), -yaxis.dotpro(eye), -zaxis.dotpro(eye), 1.0f
    };
}

//プロジェクション変換行列
Matrix4x4 Matrix4x4::perspe(float fov, float aspect, float near, float far) const {
    float h = 1.0f / tanf(fov * 0.5f);
    float w = h / aspect;
    float invRange = 1.0f / (far - near);

    return Matrix4x4{
           w, 0.0f,         0.0f, 0.0f,
        0.0f,    h,         0.0f, 0.0f,
        0.0f, 0.0f,  far * invRange, 1.0f,
        0.0f, 0.0f, -(far * near) * invRange, 0.0f
    };
}