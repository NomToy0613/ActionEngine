#include<cmath>
#include "Camera.h"
#include "Vector3.h"
#include "Matrix4x4.h"
using namespace std;

void Camera::update(const Vector3& pivot){
    switch(mode){
            case CameraMode::FirstPerson:
                updateFirstPerson();
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
void Camera::updateFirstPerson() {
    Vector3 forward(
        cosf(pitch) * sinf(yaw),
        sinf(pitch),
        cosf(pitch) * cosf(yaw)
    );

    target = eye.add(forward);
}


void Camera::updateThirdPerson(const Vector3& pivot){
    Vector3 forward(
        cosf(pitch) * sinf(yaw),
        sinf(pitch),
        cosf(pitch) * cosf(yaw)
    );
    
    target = pivot;
    eye = target.sub(forward.ply(distance));
}

void Camera::updateOverhead() {

}