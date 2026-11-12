#include<cmath>
#include "Camera.h"
#include "Vector3.h"
#include "Matrix4x4.h"
using namespace std;

Vector3 Camera::forwardDir() const{
    return Vector3(cosf(pitch) * sinf(yaw), sinf(pitch), cosf(pitch) * cosf(yaw), 0.0f);
}

Vector3 Camera::flatForward() const{
    return Vector3(sinf(yaw), 0.0f, cosf(yaw), 0.0f);
}

Vector3 Camera::flatRight() const{
    return Vector3(cosf(yaw), 0.0f, -sinf(yaw), 0.0f);
}

void Camera::update(const Vector3& pivot){
    switch(mode){
            case CameraMode::FirstPerson:
                updateFirstPerson(pivot);
                break;
            case CameraMode::ThirdPerson:
                updateThirdPerson(pivot);
                break;
            case CameraMode::Overhead:
                updateOverhead();
                break;
        }
}

Matrix4x4 Camera::view() const {
    Matrix4x4 baseM;
    baseM = baseM.identity();
    return baseM.lookat(this->eye, this->target, this->up);
}
void Camera::updateFirstPerson(const Vector3& pivot) {
    Vector3 forward = forwardDir();
    eye = pivot.add(Vector3(0, eyeHeight, 0));
    target = eye.add(forward);
}

void Camera::updateThirdPerson(const Vector3& pivot){
    Vector3 forward = forwardDir();
    target = pivot.add(Vector3(0, lookHeight, 0));
    eye = target.sub(forward.ply(distance));
}

void Camera::updateOverhead() {

}