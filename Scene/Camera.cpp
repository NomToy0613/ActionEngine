#include<cmath>
#include "Camera.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix4x4.h"
using namespace std;

void Camera::updateVectors() {
    Vector3 forward(
        cosf(pitch) * sinf(yaw),
        sinf(pitch),
        cosf(pitch) * cosf(yaw)
    );
    target = eye.add(forward);
}

Matrix4x4 Camera::view() const {
    Matrix4x4 baseM;
    baseM = baseM.identity();
    return baseM.lookat(this->eye, this->target, this->up);
}